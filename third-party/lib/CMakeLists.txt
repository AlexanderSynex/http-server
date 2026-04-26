include(FetchContent)
FetchContent_Declare(lib
    GIT_REPOSITORY #https://github.com/lib/lib.git
    GIT_TAG        #v1.1.1
)
FetchContent_MakeAvailable(lib)