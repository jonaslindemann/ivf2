if (WIN32)
    add_ivf2_example(example8 SOURCES example8.cpp)
else()
    add_ivf2_example(example8 SOURCES example8.cpp EXTRA_LIBS dl pthread)
endif()



