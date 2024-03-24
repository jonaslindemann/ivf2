#!/bin/bash

# A function to display help message
usage() {
    echo "Usage: $0 [options]"
    echo "-h, --help            Display this help message."
    echo "-r, --release         Configure/build for release."
    echo "-d, --debug           Configure/build for debug (default)."
    echo "-c, --configure       Configure only."
    echo "-b, --build           Configure and build"
    echo "--clean               Clean build dirs."
}

# A function to parse long options
parse_long_options() {
    case "$1" in
        --help)
            usage
            exit 0
            ;;
        --release)
            RELEASE=1
            shift
            ;;
        --debug)
            DEBUG=1
            shift
            ;;
        --configure)
            CONFIGURE=1
            shift
            ;;
        --build)
            BUILD=1
            CONFIGURE=1
            shift
            ;;
        --clean)
            CLEAN=1
            shift
            ;;
        *)
            echo "Unknown option: $1"
            usage
            exit 1
            ;;
    esac
}

# Default values
DEBUG=0
RELEASE=0
BUILD=0
CONFIGURE=0
CLEAN=0

DEBUG_DIR=build-debug
RELEASE_DIR=build-release


# Check if we got any arguments
if [ $# -eq 0 ]; then
    usage
    exit 1
fi

# Main loop to parse options
while [ "$#" -gt 0 ]; do
    if [[ "$1" == --* ]]; then
        parse_long_options "$@"
    else
        while getopts ":hrdcb:" opt; do
            case ${opt} in
                h )
                    usage
                    exit 0
                    ;;
                r )
                    RELEASE=1
                    DEBUG=0
                    ;;
                d )
                    DEBUG=1
                    RELEASE=0
                    ;;
                c )
                    CONFIGURE=1
                    BUILD=0
                    ;;
                b )
                    CONFIGURE=1
                    BUILD=1
                    ;;
                \? )
                    echo "Invalid option: $OPTARG" 1>&2
                    usage
                    exit 1
                    ;;
                : )
                    echo "Invalid option: $OPTARG requires an argument" 1>&2
                    exit 1
                    ;;
            esac
        done
        shift $((OPTIND -1))
    fi
    shift
done

if [ $DEBUG -eq 0 ] && [ $RELEASE -eq 0 ]; then
    DEBUG=1
fi

# Your script's logic here
if [ $DEBUG -eq 1 ]; then
    echo "-- Debug build is enabled."
fi

if [ $RELEASE -eq 1 ]; then
    echo "-- Release build is enabled."
fi

if [ $CLEAN -eq 1 ]; then
    if [ $DEBUG -eq 1 ]; then
        echo "-- Cleaning debug build dir."
        rm -rf $DEBUG_DIR
    fi

    if [ $RELEASE -eq 1 ]; then
        echo "-- Cleaning release build dir."
        rm -rf $RELEASE_DIR
    fi
fi

if [ $CONFIGURE -eq 1 ]; then

    if [ $DEBUG -eq 1 ]; then
        echo "-- Configuring debug build."
        cmake -B $DEBUG_DIR -DCMAKE_BUILD_TYPE=Debug --preset linux
    fi

    if [ $RELEASE -eq 1 ]; then
        echo "-- Configuring release build."
        cmake -B $RELEASE_DIR -DCMAKE_BUILD_TYPE=Release --preset linux
    fi
fi

if [ $BUILD -eq 1 ]; then
    if [ $DEBUG -eq 1 ]; then
        echo "-- Building debug build."
        cmake --build $DEBUG_DIR --config Debug 
    fi

    if [ $RELEASE -eq 1 ]; then
        echo "-- Building release build."
        cmake --build $DEBUG_DIR --config Release
    fi
fi


