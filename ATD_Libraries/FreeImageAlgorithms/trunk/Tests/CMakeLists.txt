SUBDIRS(FiaViewer)

SET(FIA_TEST_SRCS AllTests.cpp
                  CuTest.cpp
                  FreeImageAlgorithms_ArithmeticTests.cpp
                  FreeImageAlgorithms_DrawingTests.cpp
                  FreeImageAlgorithms_IOTests.cpp
                  FreeImageAlgorithms_LogicTests.cpp
                  FreeImageAlgorithms_ColourTests.cpp
                  FreeImageAlgorithms_ConvolutionTests.cpp
                  FreeImageAlgorithms_GradientBlend.cpp
                  FreeImageAlgorithms_FFTTests.cpp
                  FreeImageAlgorithms_LinearScale.cpp
                  FreeImageAlgorithms_MorphologyTests.cpp
                  FreeImageAlgorithms_ParticleTests.cpp
                  FreeImageAlgorithms_StatisticTests.cpp
                  FreeImageAlgorithms_UtilityTests.cpp
                  FreeImageAlgorithms_PaletteTests.cpp
)

INCLUDE_DIRECTORIES("C://Devel/ATD/ATD_Libraries/ImageViewer/trunk")

ADD_DEFINITIONS(-D IMAGEVIEWER_IMPORTS -D IMAGEVIEWER_STDCALL)

# Make sure the linker can find the Hello library once it is built.
LINK_DIRECTORIES (${FREEIMAGEALGORITHMS_BINARY_DIR} "C://dlls")

ADD_EXECUTABLE (FreeImageAlgorithmsTests ${FIA_TEST_SRCS})

# Link the executable to the Hello library.
TARGET_LINK_LIBRARIES (FreeImageAlgorithmsTests freeimagealgorithms)

#COPY_TARGET(FreeImageAlgorithmsTests)
