#pragma once
#include <memory>

#include <Common/Thread.h>
#include <Engine/SystemBase.h>
#include <Components/AudioListenerComponentFwd.h>

#include "AudioConfig.h"
#include "AudioMixerBasePtr.h"

namespace Audio
{
    class AudioSystem : public Engine::SystemBase
    {
        ROOT_OBJECT(AudioSystem)
        BASE_OBJECT(SystemBase)
        RTTR_ENABLE(SystemBase)

    public:
        friend class AudioMixer;

        AudioSystem(Engine::EngineContext* context);
        virtual ~AudioSystem();

        
        Engine::RootObject*     clone() const override {
            throw std::runtime_error("Not Implemented");
        }

        //////////////////////////////////////////////////////////////////////////
        //\Brief: Explicitly set mixer, must happen before 'init' call
        //////////////////////////////////////////////////////////////////////////
        void                    setMixer( const AudioMixerBasePtr& mixer );
        bool                    initialize( const AudioConfig& config = GetDefaultAudioConfig() );
        bool                    start();

        void                    setListener( AudioListener* listener );
        AudioListener*          getListener() const;
     
        bool                    addAudioSourceLocked(AudioSource* audio);
        bool                    removeAudioSourceLocked(AudioSource* audio);
        bool                    containsAudioSourceLocked(AudioSource* audio);
        ActiveAudioVector       getActiveSoundsLocked() const;
        bool                    removeAllAudioSourcesLocked();    

        Common::Mutex&          getSoundComponentMutex() const;

        //////////////////////////////////////////////////////////////////////////
        //\Brief: Stream in new data
        //////////////////////////////////////////////////////////////////////////
        void                    onAudioUpdate(Engine::Event& evt);

        //////////////////////////////////////////////////////////////////////////
        //\Brief: Mix active sounds, happens in background thread
        //////////////////////////////////////////////////////////////////////////
        std::uint32_t           updateAndMix( std::uint32_t numSamples, void* data );

    private:

        void                    shutDown();
        bool                    containsAudioSource(AudioSource* audio);

        mutable Common::Mutex   m_modifyActiveSoundsMutex;
        ActiveAudioSet          m_activeSounds;
        float                   m_totalAudioTime;

        class pimpl;
        std::unique_ptr<pimpl>  m_impl;
        AudioListener*          m_listener = {nullptr};        
        AudioMixerBasePtr       m_mixer;
    };
}
