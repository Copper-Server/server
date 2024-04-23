#ifndef SRC_BASE_OBJECTS_POSITION
#define SRC_BASE_OBJECTS_POSITION

union Position {
    struct {
        int x : 26;
        int z : 26;
        int y : 12;
    };

    unsigned long long raw;
};

#endif /* SRC_BASE_OBJECTS_POSITION */
