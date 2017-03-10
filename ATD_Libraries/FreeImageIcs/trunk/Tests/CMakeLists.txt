SET(FI_ICS_TEST_SRCS AllTests.c
					 CuTest.c
					 FreeImageIcs_Tests.c	  
)


ADD_EXECUTABLE (FreeImageIcsTests ${FI_ICS_TEST_SRCS})

# Make sure the linker can find the Hello library once it is built.
LINK_DIRECTORIES (${LIBRARY_OUTPUT_PATH})

# Link the executable to the Hello library.
TARGET_LINK_LIBRARIES (FreeImageIcsTests FreeImageIcs
										 ${FREEIMAGEALGORITHMS_LIBRARY}
										 ${FREEIMAGE_LIBRARY})
