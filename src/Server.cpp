#include <cstring>
#include <iostream>
#include <fstream>
#include <AudioToolbox/AudioToolbox.h>

int WAV_HEADER_SIZE = 40;

struct wav_file_information_t {
    char description_header[5]; 
    uint32_t page_size; 
    char wav_description_header[5];
    char fmt_description_header[5];
    uint32_t section_size_chunk;
    uint16_t audio_format;
    uint16_t mono_stereo_flag;
    uint32_t sample_frequency;
    uint32_t byte_rate; // per sec;
    uint16_t block_align;
    uint16_t bits_per_sample;
    char data_description_header[5];
    uint32_t data_size;
};

struct audio_data_t {
    const char* data;
    uint32_t size;
    uint32_t position;
};

void AudioCallback(void* custom_data, AudioQueueRef queue, AudioQueueBufferRef buffer) {
    audio_data_t* audio_data = static_cast<audio_data_t*>(custom_data);

    uint32_t bytesToCopy = std::min(buffer->mAudioDataBytesCapacity, audio_data->size - audio_data->position);
    std::memcpy(buffer->mAudioData, audio_data->data + audio_data->position, bytesToCopy);

    buffer->mAudioDataByteSize = bytesToCopy;
    audio_data->position += bytesToCopy;

    AudioQueueEnqueueBuffer(queue, buffer, 0, nullptr);

    if (audio_data->position >= audio_data->size) {
        AudioQueueStop(queue, false);
    }
}

void RewriteDataBufferToSineWave(char *buffer, wav_file_information_t fileInfo) {
    int sample_rate = fileInfo.sample_frequency;  // e.g., 44100 Hz
    int bufferLength = fileInfo.data_size;  // Length of the buffer in bytes
    int sine_frequency = 440;  // Frequency of the sine wave (e.g., 440 Hz for A4 note)
    double modulation_frequency = 0.5;  // Frequency of the volume modulation (0.5 Hz)

    for (int i = 0; i < bufferLength; i++) {
        // Calculate the current time in seconds
        double time = i / static_cast<double>(sample_rate);

        // Calculate the volume modulation factor using a sine wave
        double volume_modulation = 0.5 * (2.0 + sin(2.0 * M_PI * modulation_frequency * time));

        // Calculate the sine wave sample
        double sine_sample = sin(2.0 * M_PI * sine_frequency * time);

        // Apply the volume modulation to the sine wave sample
        double sample = volume_modulation * sine_sample;

        // Scale and convert the sample to an unsigned char (0-255 range)
        buffer[i] = static_cast<unsigned char>(10.0 * (sample + 1.0));
    }
}

int main(int argc, char* argv[]) {
    // Flush after every std::cout / std::cerr
    std::cout << std::unitbuf;
    std::cerr << std::unitbuf;

    // You can use print statements as follows for debugging, they'll be visible when running tests.
    std::cout << "Logs from your program will appear here" << std::endl;

    if (argc != 2) {
        std::cerr << "Expected two arguments" << std::endl;
        return 1;
    }

    std::string database_file_path = argv[1];

    std::ifstream database_file(database_file_path, std::ios::binary);
    if (!database_file)
    {
        std::cerr << "Failed to open the database file" << std::endl;
        return 1;
    }

    wav_file_information_t wav_file_info = {0};

    std::cout << "size of wav_file_information: " << sizeof(wav_file_information_t) << std::endl;

    // description header
    database_file.read(wav_file_info.description_header, 4);
    wav_file_info.description_header[4] = '\0';

    // page size
    database_file.read(reinterpret_cast<char*>(&wav_file_info.page_size), 4);

    // wav description header
    database_file.read(wav_file_info.wav_description_header, 4);
    wav_file_info.wav_description_header[4] = '\0';

    // fmt description header
    database_file.read(wav_file_info.fmt_description_header, 4);
    wav_file_info.fmt_description_header[4] = '\0';

    // section chunk size
    database_file.read(reinterpret_cast<char*>(&wav_file_info.section_size_chunk), 4);

    // audio format
    database_file.read(reinterpret_cast<char*>(&wav_file_info.audio_format), 2);

    // mono stereo flag
    database_file.read(reinterpret_cast<char*>(&wav_file_info.mono_stereo_flag), 2);

    // sample frequency
    database_file.read(reinterpret_cast<char*>(&wav_file_info.sample_frequency), 4);

    // byte rate
    database_file.read(reinterpret_cast<char*>(&wav_file_info.byte_rate), 4);

    // block align
    database_file.read(reinterpret_cast<char*>(&wav_file_info.block_align), 2);

    // bits per sample
    database_file.read(reinterpret_cast<char*>(&wav_file_info.bits_per_sample), 2);

    // data description header
    database_file.read(wav_file_info.description_header, 4);
    wav_file_info.description_header[4] = '\0';

    // data size
    database_file.read(reinterpret_cast<char*>(&wav_file_info.data_size), 4);

    // Data
    char data[wav_file_info.data_size];
    database_file.read(data, wav_file_info.data_size);

    std::cout << "\n\n====================================" << std::endl;
    std::cout << "======= WAV FILE INFORMATION =======" << std::endl;
    std::cout << "====================================\n" << std::endl;
    std::cout << "file description header: " << wav_file_info.description_header << std::endl;
    std::cout << "file page size: " << wav_file_info.page_size<< std::endl;
    std::cout << "file wav description header: " << wav_file_info.wav_description_header << std::endl;
    std::cout << "file fmt description header: " << wav_file_info.fmt_description_header << std::endl;
    std::cout << "file section size chunk: " << wav_file_info.section_size_chunk << std::endl;
    std::cout << "file audio format: " << wav_file_info.audio_format << std::endl;
    std::cout << "file mono stereo flag: " << wav_file_info.mono_stereo_flag << std::endl;
    std::cout << "file sample frequency: " << wav_file_info.sample_frequency << std::endl;
    std::cout << "file byte rate: " << wav_file_info.byte_rate << std::endl;
    std::cout << "file block align: " << wav_file_info.block_align << std::endl;
    std::cout << "file bits per sample: " << wav_file_info.bits_per_sample << std::endl;
    std::cout << "file data description header: " << wav_file_info.data_description_header << std::endl;
    std::cout << "file data size: " << wav_file_info.data_size << std::endl;
    std::cout << "\n============= DATA ================\n" << std::endl;
    std::cout << "data size: " << sizeof(data) << std::endl;

    // Uncomment to rewrite to sine wave
    // RewriteDataBufferToSineWave(data, wav_file_info);

    // Play audio
    AudioStreamBasicDescription format;
    format.mSampleRate = wav_file_info.sample_frequency;
    format.mFormatID = kAudioFormatLinearPCM; 
    format.mFormatFlags = kAudioFormatFlagIsSignedInteger | kAudioFormatFlagIsPacked;
    format.mFramesPerPacket = 1;
    format.mChannelsPerFrame = wav_file_info.mono_stereo_flag;
    format.mBitsPerChannel = wav_file_info.bits_per_sample;
    format.mBytesPerPacket = format.mBytesPerFrame = wav_file_info.bits_per_sample / 8;
    format.mReserved = 0;

    // Set up the audio data structure
    audio_data_t audio_data;
    audio_data.data = data;
    audio_data.size = sizeof(data);
    audio_data.position = 0;

    // Create the audio queue
    AudioQueueRef queue;
    OSStatus outputStatus = AudioQueueNewOutput(&format, AudioCallback, &audio_data, nullptr, nullptr, 0, &queue);
    if (outputStatus != noErr) {
        std::cerr << "Error creating AudioQueue output: " << outputStatus << std::endl;
        return -1;
    }

    // Allocate and prime the buffer
    AudioQueueBufferRef buffer;
    OSStatus status = AudioQueueAllocateBuffer(queue, wav_file_info.data_size, &buffer);
    if(status != noErr) {
        std::cerr << "Failed to allocate buffer" << std::endl;
        std::cerr << "Error code: " << status << std::endl;
        return -1;
    }
    AudioCallback(&audio_data, queue, buffer); // Initial filling of the buffer

    std::cout << "Setting up start audio playback" << std::endl;
    // Start playback
    AudioQueueStart(queue, nullptr);

    std::cout << "Playing audio. Press Enter to stop..." << std::endl;
    std::cin.get(); // Wait for user input

    std::cout << "cleanuip" << std::endl;
    // Clean up
    AudioQueueStop(queue, true);
    AudioQueueDispose(queue, true);

    database_file.close();
    return 0;
}
