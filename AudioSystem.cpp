#include <chrono>
#define MINI_AL_IMPLEMENTATION
#include <al/mini_al.h>
#include <Common/ReflectionRegister.h>
#include <Common/Thread.h>
#include <Math/GenMath.h>
#include <Profiler/Profiler.h>
#include <Engine/EngineContext.h>
#include <Engine/DefaultEvents.h>
#include <Engine/EventSystem.h>
#include <Engine/Event.h>


#include <Components/AudioListenerComponent.h>
#include <Components/AudioComponent.h>

#include <Console/Logger.h>
#include <Console/LogLevel.h>

#include <Scene/NodeSystem.h>
#include <Scene/AudioEmitterEntity.h>
#include <Scene/AudioListenerEntity.h>

#include "AudioException.h"
#include "AudioMixerDefault.h"
#include "AudioSystem.h"

using namespace Components;
using namespace Common;
using namespace Engine;
using namespace Log;
using namespace Math;
using namespace std::chrono_literals;
using namespace Scene;


RTTR_REGISTRATION
{
    using namespace Audio;
    using namespace rttr;
    using namespace Engine;
    auto val = registration::class_<AudioSystem>("AudioSystem");
    val.method("addAudioSource", &AudioSystem::addAudioSourceLocked);
    val.method("removeAudioSource", &AudioSystem::removeAllAudioSourcesLocked);
    val.method("containsAudioSource", &AudioSystem::containsAudioSourceLocked);
    val.method("removeAllAudioSources", &AudioSystem::removeAllAudioSourcesLocked);
    val.method("getActiveSounds", &AudioSystem::getActiveSoundsLocked);

    val.method("setListener", &AudioSystem::setListener);
    val.method("getListener", &AudioSystem::getListener);
    val.method("setMixer", &AudioSystem::setMixer);
}

namespace Audio
{
    
    void LogMessageOnFailAndThrow( Logger* logger, const std::string& msg, mal_result result)
    {
        if ( MAL_SUCCESS != result )
        {
            logger->addMessage( msg.c_str(), LOG_LEVEL_ERROR );
            throw AudioException( msg );
        }
    }

    mal_uint32 onSendFramesToDevice( mal_device* pDevice, mal_uint32 sampleCount, void* pSamples )
    {
        auto* audioSys =  reinterpret_cast<AudioSystem*>(pDevice->pUserData);
        return audioSys->updateAndMix(sampleCount, pSamples);
    }

    inline  void onDeviceStop( mal_device* devicePtr )
    {
        (devicePtr);
    }    

    void onAudiolog(mal_context* pContext, mal_device* pDevice, const char* message)
    {
        (void)pContext;
        (void)pDevice;
        printf("mini_al: %s\n", message);
    }

#pragma region pimpl
    //////////////////////////////////////////////////////////////////////////
    //\Brief: AudioSystem implementation
    //////////////////////////////////////////////////////////////////////////
    class AudioSystem::pimpl
    {
        friend class AudioSystem;
    public:
        pimpl(EngineContext* context)
            : m_context(context)
            , m_initialized(false)
            , m_running( false )
        {
        }

        ~pimpl()
        {
            if( m_running )
                shutDown();
        }

        bool start()
        {
            if (m_running)
                throw AudioException("Audio System Already Started");
            
            using namespace Log;
            const auto& logger = m_context->getSystem<Logger>();
            
            if (mal_device_start(&m_playBackDevice) != MAL_SUCCESS) {
                logger->addMessage("Audio Device Failed", LOG_LEVEL_ERROR);
                return false;
            }
            logger->addMessage("Audio Device Started", LOG_LEVEL_SUCCES);
            m_running = true;           
            return true;
        }

        void shutDown()
        {
            if (!m_running)
               return;

            mal_device_uninit(&m_playBackDevice);
            mal_context_uninit(&m_audioContext);
            m_running = false;
        }

        bool initialize( const AudioConfig& config )
        {
            m_outputFormat = config;
            const auto& logger   = m_context->getSystem<Logger>();
            const auto& audioSys = m_context->getSystem<AudioSystem>();
                 
            logger->addMessage("Initializing Audio System", LOG_LEVEL_INFO );
            logger->addMessage("Audio Thread Id: " + std::to_string(Common::GetThreadId()));

            //init context  config
            m_audioContextConfig = mal_context_config_init(onAudiolog);

            //determine send callback
            mal_send_proc sendCb = onSendFramesToDevice;

            //init device
            m_audioDeviceConfig = mal_device_config_init_playback((mal_format)m_outputFormat.m_format,
                m_outputFormat.m_channels, m_outputFormat.m_sampleRate, sendCb );
            m_audioDeviceConfig.onStopCallback = onDeviceStop;

            LogMessageOnFailAndThrow(logger, "mal_context_init",        mal_context_init(nullptr, 0, &m_audioContextConfig, &m_audioContext));
            LogMessageOnFailAndThrow(logger, "mal_context_get_devices", mal_context_get_devices( &m_audioContext, &m_playbackDeviceInfos, &m_playbackDeviceCount,
                &m_captureDeviceInfos, &m_captureDeviceCount));

            //print device names
            for (mal_uint32 i = 0; i < m_playbackDeviceCount; ++i) {
                std::string deviceName = m_playbackDeviceInfos[i].name;
                std::string msg = "Audio Device: " + std::to_string(i) + " " + deviceName;
                logger->addMessage(msg.c_str(), (int)eLogLevel::LOG_LEVEL_INFO);
            }

            //init sound generation
            void* soundGen = audioSys;
            
            //init playback
            LogMessageOnFailAndThrow(logger, "mal_device_init failed",
                mal_device_init(nullptr, mal_device_type_playback, nullptr, &m_audioDeviceConfig, soundGen, &m_playBackDevice));
            m_initialized = true;                
            logger->addMessage("Audio System Initialized", LOG_LEVEL_SUCCES );
            return m_initialized;
        }
        bool                m_initialized;
        std::atomic<bool>   m_running;

        mal_sine_wave       m_sineWave;       
        mal_context         m_audioContext;
        mal_context_config  m_audioContextConfig;

        mal_decoder         m_decoder;

        mal_device_info*    m_playbackDeviceInfos;
        mal_uint32          m_playbackDeviceCount;

        mal_device_info*    m_captureDeviceInfos;
        mal_uint32          m_captureDeviceCount;

        mal_device_config   m_audioDeviceConfig;
        mal_device          m_playBackDevice;

        AudioConfig         m_outputFormat;   
        EngineContext*      m_context;
    };
#pragma endregion



    //////////////////////////////////////////////////////////////////////////
    //\@AudioSystem implementation
    //////////////////////////////////////////////////////////////////////////
    AudioSystem::AudioSystem(EngineContext* context)
        : SystemBase(context)
        , m_impl(  std::make_unique<AudioSystem::pimpl>(context))       
        , m_mixer( std::make_shared<MixerDefault>(context))
        , m_totalAudioTime( 0.0f )
    {
        
        subscribeToEvent(CreateEventHandler( this, &AudioSystem::onAudioUpdate, "AUDIO_UPDATE"));
    }

    AudioSystem::~AudioSystem()
    {
        AudioSystem::shutDown();
    }


    bool AudioSystem::initialize(const AudioConfig& config)
    {
        assert(config.m_format == eAudioFormat::audio_format_f32);
        
        if (!m_mixer)
            throw AudioException("No Mixer Specified!");

        return m_mixer->initialize( config ) && 
               m_impl->initialize( config );              
    }

    bool AudioSystem::start()
    {
       return m_impl->start();
    }

  
    Common::Mutex& AudioSystem::getSoundComponentMutex() const
    {
        return m_modifyActiveSoundsMutex;
    }

    ActiveAudioVector AudioSystem::getActiveSoundsLocked() const
    {
        LockGuard lock( m_modifyActiveSoundsMutex );
        ActiveAudioVector result;
        result.reserve(m_activeSounds.size());
        for (const auto& as : m_activeSounds)
            result.emplace_back(as);
        return result;
    }

    bool AudioSystem::removeAllAudioSourcesLocked()
    {
        LockGuard lock(m_modifyActiveSoundsMutex);
        m_activeSounds.clear();
        return true;
    }

    
    void AudioSystem::setListener(AudioListener* listener)
    {
        m_listener = listener;
    }

    AudioListener* AudioSystem::getListener() const
    {
         return  m_listener;
    }

    void AudioSystem::setMixer(const AudioMixerBasePtr& mixer)
    {
        m_mixer = mixer;
    }

    bool AudioSystem::addAudioSourceLocked( AudioSource* audio )
    {
        LockGuard lock(m_modifyActiveSoundsMutex);
        if (containsAudioSource(audio))
            return false;
        m_activeSounds.insert(audio);
        return true;
    }

    bool AudioSystem::removeAudioSourceLocked( AudioSource* audio )
    {
        LockGuard lock(m_modifyActiveSoundsMutex);
        if (!containsAudioSource(audio))
            return false;      
        audio->pause();
        m_activeSounds.erase(audio);
        return true;
    }

    void AudioSystem::onAudioUpdate(Engine::Event& evt)
    {
        auto frameTime = evt.getValue<float>("AUDIO_TIME_STEP");
        for (const auto& it : m_activeSounds)
            it->onAudioUpdate(frameTime);
        m_totalAudioTime += frameTime;
    }

    std::uint32_t AudioSystem::updateAndMix(std::uint32_t numSamples, void* data)
    {
        //create a copy of current active sounds & work with that
        auto aav = getActiveSoundsLocked();
        if (!m_mixer->updateActiveSounds( aav ) )
            return false;
        return m_mixer->mixIncomingSounds( aav, numSamples, data );
    }

    void AudioSystem::shutDown()
    {
        LockGuard lock(m_modifyActiveSoundsMutex);
        m_activeSounds.clear();
    }

    bool AudioSystem::containsAudioSource(AudioSource* audio)
    {
        auto it = m_activeSounds.find(audio);
        return std::end(m_activeSounds) != it;
    }

    bool AudioSystem::containsAudioSourceLocked(AudioSource* audio)
    {
        LockGuard lock(m_modifyActiveSoundsMutex);
        auto it = m_activeSounds.find(audio);
        return std::end(m_activeSounds) != it;
    }

}