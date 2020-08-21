#pragma once
#include <array>
#include "AudioConfig.h"
namespace Audio
{
    template<int BUF_SIZE>
    struct alignas(16) AudioBlock
    {
        AudioBlock()          
            : m_dataPos(0)
            , m_totalSamples( 0 )
        {       
            clear();
        }

        AudioBlock(const AudioBlock<BUF_SIZE>& rhs)
            : m_data(rhs.m_data)
            , m_dataPos(rhs.m_dataPos)
            , m_totalSamples( rhs.m_totalSamples)
        {

        }

        inline void                rewind()
        {
            setPosition(0);
        }

        void                       clear()
        {
            memset( m_data.data(), 0, getByteSize() );
        }

        inline char*               getData() {
            return &m_data[m_dataPos];
        }

        inline const char*         getData() const {
            return &m_data[m_dataPos];
        }

        inline constexpr int       getByteSize() const {
            return BUF_SIZE;
        }

        inline std::uint32_t       availableBytes() const {
            return BUF_SIZE - m_dataPos;
        }

        inline void                setPosition(std::uint32_t pos) {
            m_dataPos = pos;
        }

        inline const std::uint32_t getPosition() const {
            return m_dataPos;
        }

        template<typename T>
        inline T*                  toPointer() 
        {
            return reinterpret_cast<T*>( m_data.data() );
        }

        template<typename T>
        inline const T*            toConstPointer() const
        {
            return reinterpret_cast<const T*>( m_data.data() );
        }

        /*
            @brief: Write data to block, returns # bytes written
        */                           
        inline std::uint32_t       writeData( const void* data, std::uint32_t numBytes ) {
            const auto actualBytes = std::min( numBytes, availableBytes() );
            if ( actualBytes == 0 )
                return actualBytes;

            auto* destPos = &m_data[m_dataPos];            
            memcpy( destPos, data, actualBytes );    
            m_dataPos += actualBytes;
            return actualBytes;            
        }

        /*
            @brief: Read data from block, returns # bytes read
        */                          
        inline std::uint32_t       readData( void* dest, std::uint32_t numBytes)  {
            const auto actualBytes = std::min( numBytes, availableBytes() );
            if ( actualBytes == 0 )
                return actualBytes;

            const auto* srcPos = &m_data[m_dataPos];            
            memcpy(dest, srcPos, actualBytes);
            m_dataPos += actualBytes;
            return actualBytes;
        }
        static constexpr std::uint32_t  BUF_MASK = BUF_SIZE - 1;       
        std::array<char, BUF_SIZE>      m_data;       
        std::uint32_t                   m_dataPos;
        std::uint32_t                   m_totalSamples;
        std::uint32_t                   m_padding[2];
    };

    using AudioBlock1k  = AudioBlock<1024>;
    using AudioBlock2k  = AudioBlock<2048>;
    using AudioBlock4k  = AudioBlock<4096>;
    using AudioBlock8k  = AudioBlock<8192>;
    using AudioBlock16k = AudioBlock<16384>;
}