cmake_minimum_required(VERSION 3.15)
project(GhostSurf VERSION 1.0.0)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# JUCE must be cloned into ./JUCE
# git clone https://github.com/juce-framework/JUCE.git
add_subdirectory(JUCE)

juce_add_plugin(GhostSurf
    COMPANY_NAME            "WipeOut Audio"
    IS_SYNTH                FALSE
    NEEDS_MIDI_INPUT        FALSE
    NEEDS_MIDI_OUTPUT       FALSE
    IS_MIDI_EFFECT          FALSE
    EDITOR_WANTS_KEYBOARD_FOCUS FALSE
    COPY_PLUGIN_AFTER_BUILD FALSE
    PLUGIN_MANUFACTURER_CODE Wipo
    PLUGIN_CODE             Gsrf
    FORMATS                 VST3 Standalone
    PRODUCT_NAME            "GhostSurf"
    VERSION                 "1.0.0"
    PLUGIN_DESCRIPTION      "Vintage Surf Rock Effects"
)

target_sources(GhostSurf PRIVATE
    Source/PluginProcessor.cpp
    Source/PluginEditor.cpp
)

target_compile_definitions(GhostSurf
    PUBLIC
        JUCE_WEB_BROWSER=0
        JUCE_USE_CURL=0
        JUCE_VST3_CAN_REPLACE_VST2=0
        JUCE_DISPLAY_SPLASH_SCREEN=0
        JUCE_REPORT_APP_USAGE=0
)

target_link_libraries(GhostSurf
    PRIVATE
        juce::juce_audio_utils
        juce::juce_dsp
    PUBLIC
        juce::juce_recommended_config_flags
        juce::juce_recommended_lto_flags
        juce::juce_recommended_warning_flags
)
