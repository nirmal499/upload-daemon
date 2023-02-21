add_library(request src/request.cpp)

target_include_directories(request
    PUBLIC include
    PRIVATE ../external/cpr/include
    PRIVATE ../external/json/include
)

target_link_libraries(request
    PRIVATE nlohmann_json::nlohmann_json
    PRIVATE cpr::cpr
)