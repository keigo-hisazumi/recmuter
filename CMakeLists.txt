cmake_minimum_required(VERSION 3.22)

project(RecMuter VERSION 0.1.0 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# macOS では Intel / Apple Silicon 両対応のユニバーサルバイナリを作る
if(APPLE AND NOT CMAKE_OSX_ARCHITECTURES)
    set(CMAKE_OSX_ARCHITECTURES "arm64;x86_64" CACHE STRING "" FORCE)
endif()
set(CMAKE_OSX_DEPLOYMENT_TARGET "11.0" CACHE STRING "")

option(RECMUTER_BUILD_TESTS "ミュート判定ロジックの単体テストをビルドする" ON)

# ---------------------------------------------------------------------------
# JUCE の取得
#   JUCE 8 には AAX SDK が同梱されているため、別途 AAX SDK を用意する必要はない。
#   ローカルの JUCE を使う場合は -DRECMUTER_JUCE_DIR=/path/to/JUCE を指定する。
# ---------------------------------------------------------------------------
set(RECMUTER_JUCE_DIR "" CACHE PATH "ローカルの JUCE ディレクトリ（空なら自動取得）")

if(RECMUTER_JUCE_DIR)
    add_subdirectory("${RECMUTER_JUCE_DIR}" JUCE EXCLUDE_FROM_ALL)
else()
    include(FetchContent)
    FetchContent_Declare(JUCE
        GIT_REPOSITORY https://github.com/juce-framework/JUCE.git
        GIT_TAG 8.0.12
        GIT_SHALLOW TRUE)
    FetchContent_MakeAvailable(JUCE)
endif()

# ---------------------------------------------------------------------------
# プラグイン本体
#   FORMATS のうち、ビルド環境で対応していない形式は JUCE が自動的に除外する。
#   （AAX は macOS / Windows x64 のみ。Linux では VST3 のみビルドされる）
# ---------------------------------------------------------------------------
juce_add_plugin(RecMuter
    COMPANY_NAME "RecMuter"
    BUNDLE_ID "com.recmuter.RecMuter"
    PLUGIN_MANUFACTURER_CODE Rcmt
    PLUGIN_CODE RcMt
    PRODUCT_NAME "RecMuter"
    FORMATS AAX VST3 AU
    AAX_CATEGORY Dynamics
    VST3_CATEGORIES Fx Tools
    AU_MAIN_TYPE kAudioUnitType_Effect
    IS_SYNTH FALSE
    NEEDS_MIDI_INPUT FALSE
    NEEDS_MIDI_OUTPUT FALSE
    IS_MIDI_EFFECT FALSE
    EDITOR_WANTS_KEYBOARD_FOCUS FALSE
    COPY_PLUGIN_AFTER_BUILD FALSE)

target_sources(RecMuter
    PRIVATE
        Source/PluginProcessor.cpp
        Source/PluginEditor.cpp)

target_include_directories(RecMuter PRIVATE Source)

target_compile_definitions(RecMuter
    PUBLIC
        JUCE_WEB_BROWSER=0
        JUCE_USE_CURL=0
        JUCE_VST3_CAN_REPLACE_VST2=0
        JUCE_DISPLAY_SPLASH_SCREEN=0)

target_link_libraries(RecMuter
    PRIVATE
        juce::juce_audio_utils
    PUBLIC
        juce::juce_recommended_config_flags
        juce::juce_recommended_lto_flags
        juce::juce_recommended_warning_flags)

# ---------------------------------------------------------------------------
# 単体テスト（JUCE に依存しない純粋なロジックのみを対象とする）
# ---------------------------------------------------------------------------
if(RECMUTER_BUILD_TESTS)
    enable_testing()
    add_executable(MuteLogicTests Tests/MuteLogicTests.cpp)
    target_include_directories(MuteLogicTests PRIVATE Source)
    add_test(NAME MuteLogicTests COMMAND MuteLogicTests)
endif()
