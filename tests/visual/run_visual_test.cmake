# Runs one example hidden, saves its last frame and compares it with the reference image.
# Called by CTest (see tests/visual/CMakeLists.txt) as
#
#   cmake -DNAME=... -DEXE=... -DWORKDIR=... -DCOMPARE=... -DREFERENCE_DIR=... -DOUTPUT_DIR=...
#         -DFRAMES=... -DTIMESTEP=... -DTHRESHOLD=... -DMAX_FRACTION=... -P run_visual_test.cmake
#
# With the environment variable IVF2_UPDATE_REFERENCES=1 the rendered image replaces the
# reference instead of being compared with it.

foreach(var NAME EXE WORKDIR COMPARE REFERENCE_DIR OUTPUT_DIR FRAMES TIMESTEP THRESHOLD MAX_FRACTION)
    if(NOT DEFINED ${var})
        message(FATAL_ERROR "run_visual_test.cmake: ${var} not set")
    endif()
endforeach()

set(actual "${OUTPUT_DIR}/${NAME}.png")
set(diff "${OUTPUT_DIR}/${NAME}_diff.png")
set(reference "${REFERENCE_DIR}/${NAME}.png")
file(MAKE_DIRECTORY "${OUTPUT_DIR}")
file(REMOVE "${actual}" "${diff}")

# Render: hidden window, fixed time step, fixed random seed, last frame without the UI
set(ENV{IVF2_HIDE_WINDOWS} 1)
set(ENV{IVF2_EXIT_AFTER_FRAMES} ${FRAMES})
set(ENV{IVF2_FIXED_TIMESTEP} ${TIMESTEP})
set(ENV{IVF2_RANDOM_SEED} 1)
set(ENV{IVF2_SCREENSHOT} "${actual}")
set(ENV{IVF2_SCREENSHOT_UI} 0)
set(ENV{IVF2_GL_DEBUG} 0)

execute_process(COMMAND "${EXE}" WORKING_DIRECTORY "${WORKDIR}" RESULT_VARIABLE result
                OUTPUT_VARIABLE output ERROR_VARIABLE output TIMEOUT 300)
if(NOT result EQUAL 0)
    message("${output}")
    message(FATAL_ERROR "${NAME} exited with ${result}")
endif()
if(NOT EXISTS "${actual}")
    message("${output}")
    message(FATAL_ERROR "${NAME} wrote no screenshot")
endif()
# Logger lines are "HH:MM:SS.mmm LEVEL Context message"
if(output MATCHES "[0-9] ERROR ")
    message("${output}")
    message(FATAL_ERROR "${NAME} logged errors")
endif()

if("$ENV{IVF2_UPDATE_REFERENCES}" STREQUAL "1")
    file(COPY_FILE "${actual}" "${reference}")
    message("Updated reference ${reference}")
    return()
endif()

if(NOT EXISTS "${reference}")
    message(FATAL_ERROR "No reference image ${reference}.\n"
                        "Rendered image: ${actual} (run with IVF2_UPDATE_REFERENCES=1 to accept it)")
endif()

execute_process(COMMAND "${COMPARE}" "${reference}" "${actual}" --diff "${diff}"
                        --threshold ${THRESHOLD} --max-fraction ${MAX_FRACTION}
                RESULT_VARIABLE result)
if(NOT result EQUAL 0)
    message(FATAL_ERROR "${NAME} differs from ${reference}\n  rendered: ${actual}\n  diff:     ${diff}")
endif()
