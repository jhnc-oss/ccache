SUITE_profiling_gcc_PROBE() {
    if ! $COMPILER_TYPE_GCC; then
        echo "compiler is not GCC"
    fi
}

SUITE_profiling_gcc_SETUP() {
    echo 'int main(void) { return 0; }' >test.c
    unset CCACHE_NODIRECT
}

SUITE_profiling_gcc() {
    # -------------------------------------------------------------------------
    TEST "-fbranch-probabilities"

    $CCACHE_COMPILE -fprofile-generate -c test.c
    expect_stat direct_cache_hit 0
    expect_stat cache_miss 1

    $COMPILER -fprofile-generate test.o -o test

    ./test

    $CCACHE_COMPILE -fbranch-probabilities -c test.c 2>/dev/null
    expect_stat direct_cache_hit 0
    expect_stat cache_miss 2

    $CCACHE_COMPILE -fbranch-probabilities -c test.c 2>/dev/null
    expect_stat direct_cache_hit 1
    expect_stat cache_miss 2

    ./test

    $CCACHE_COMPILE -fbranch-probabilities -c test.c 2>/dev/null
    expect_stat direct_cache_hit 1
    expect_stat cache_miss 3

    # -------------------------------------------------------------------------
    if [ -z "${WIN_XFAIL}" ]; then
        TEST "-fprofile-dir=dir + -fprofile-use"

        mkdir data

        $CCACHE_COMPILE -fprofile-dir=data -fprofile-generate -c test.c
        expect_stat direct_cache_hit 0
        expect_stat cache_miss 1

        $COMPILER -fprofile-generate test.o -o test

        ./test

        $CCACHE_COMPILE -fprofile-dir=data -fprofile-use -c test.c
        expect_stat direct_cache_hit 0
        expect_stat cache_miss 2

        $CCACHE_COMPILE -fprofile-dir=data -fprofile-use -c test.c
        expect_stat direct_cache_hit 1
        expect_stat cache_miss 2

        ./test

        $CCACHE_COMPILE -fprofile-dir=data -fprofile-use -c test.c
        expect_stat direct_cache_hit 1
        expect_stat cache_miss 3
    fi
    # -------------------------------------------------------------------------
    if [ -z "${WIN_XFAIL}" ]; then
        TEST "-fprofile-use + -fprofile-dir=dir"

        mkdir data

        $CCACHE_COMPILE -fprofile-generate -fprofile-dir=data -c test.c
        expect_stat direct_cache_hit 0
        expect_stat cache_miss 1

        $COMPILER -fprofile-generate test.o -o test

        ./test

        $CCACHE_COMPILE -fprofile-use -fprofile-dir=data -c test.c
        expect_stat direct_cache_hit 0
        expect_stat cache_miss 2

        $CCACHE_COMPILE -fprofile-use -fprofile-dir=data -c test.c
        expect_stat direct_cache_hit 1
        expect_stat cache_miss 2

        ./test

        $CCACHE_COMPILE -fprofile-use -fprofile-dir=data -c test.c
        expect_stat direct_cache_hit 1
        expect_stat cache_miss 3
    fi
    # -------------------------------------------------------------------------
    if [ -z "${WIN_XFAIL}" ]; then
        TEST "-fprofile-dir=path1 + -fprofile-use=path2"

        mkdir data

        $CCACHE_COMPILE -fprofile-dir=data2 -fprofile-generate=data -c test.c
        expect_stat direct_cache_hit 0
        expect_stat cache_miss 1

        $COMPILER -fprofile-generate test.o -o test

        ./test

        $CCACHE_COMPILE -fprofile-dir=data2 -fprofile-use=data -c test.c
        expect_stat direct_cache_hit 0
        expect_stat cache_miss 2

        $CCACHE_COMPILE -fprofile-dir=data2 -fprofile-use=data -c test.c
        expect_stat direct_cache_hit 1
        expect_stat cache_miss 2

        ./test

        $CCACHE_COMPILE -fprofile-dir=data2 -fprofile-use=data -c test.c
        expect_stat direct_cache_hit 1
        expect_stat cache_miss 3
    fi
}
