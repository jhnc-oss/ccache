# This test suite verified both the file storage backend and the secondary
# storage framework itself.

#SUITE_secondary_file_PROBE() {
 #   if ! $RUN_WIN_XFAIL; then
 #       echo "secondary_file tests are broken on Windows"
 #       return
 #   fi
#}

SUITE_secondary_file_SETUP() {
    unset CCACHE_NODIRECT
    if $HOST_OS_WINDOWS; then
        export MSYS2_ENV_CONV_EXCL=CCACHE_SECONDARY_STORAGE
        export CCACHE_SECONDARY_STORAGE="file:///$(cygpath -m $PWD/secondary)"
    else
        export CCACHE_SECONDARY_STORAGE="file:$PWD/secondary"
    fi
    generate_code 1 test.c
}

SUITE_secondary_file() {
    # -------------------------------------------------------------------------
    TEST "Base case"

    # Compile and send result to primary and secondary storage.
    $CCACHE_COMPILE -c test.c
    expect_stat direct_cache_hit 0
    expect_stat cache_miss 1
    expect_stat files_in_cache 2
    expect_stat primary_storage_hit 0
    expect_stat primary_storage_miss 2 # result + manifest
    expect_stat secondary_storage_hit 0
    expect_stat secondary_storage_miss 2 # result + manifest
    expect_exists secondary/CACHEDIR.TAG
    subdirs=$(find secondary -type d | wc -l)
    if [ "${subdirs}" -lt 2 ]; then # "secondary" itself counts as one
        test_failed "Expected subdirectories in secondary"
    fi
    expect_file_count 3 '*' secondary # CACHEDIR.TAG + result + manifest

    # Get result from primary storage.
    $CCACHE_COMPILE -c test.c
    expect_stat direct_cache_hit 1
    expect_stat cache_miss 1
    expect_stat primary_storage_hit 2 # result + manifest
    expect_stat primary_storage_miss 2 # result + manifest
    expect_stat secondary_storage_hit 0
    expect_stat secondary_storage_miss 2 # result + manifest
    expect_stat files_in_cache 2
    expect_file_count 3 '*' secondary # CACHEDIR.TAG + result + manifest

    # Clear primary storage.
    $CCACHE -C >/dev/null
    expect_stat files_in_cache 0
    expect_file_count 3 '*' secondary # CACHEDIR.TAG + result + manifest

    # Get result from secondary storage, copying it to primary storage.
    $CCACHE_COMPILE -c test.c
    expect_stat direct_cache_hit 2
    expect_stat cache_miss 1
    expect_stat primary_storage_hit 2
    expect_stat primary_storage_miss 4 # 2 * (result + manifest)
    expect_stat secondary_storage_hit 2 # result + manifest
    expect_stat secondary_storage_miss 2 # result + manifest
    expect_stat files_in_cache 2 # fetched from secondary
    expect_file_count 3 '*' secondary # CACHEDIR.TAG + result + manifest

    # Get result from primary storage again.
    $CCACHE_COMPILE -c test.c
    expect_stat direct_cache_hit 3
    expect_stat cache_miss 1
    expect_stat primary_storage_hit 4
    expect_stat primary_storage_miss 4 # 2 * (result + manifest)
    expect_stat secondary_storage_hit 2 # result + manifest
    expect_stat secondary_storage_miss 2 # result + manifest
    expect_stat files_in_cache 2 # fetched from secondary
    expect_file_count 3 '*' secondary # CACHEDIR.TAG + result + manifest

    # -------------------------------------------------------------------------
    TEST "Flat layout"

    CCACHE_SECONDARY_STORAGE+="|layout=flat"

    $CCACHE_COMPILE -c test.c
    expect_stat direct_cache_hit 0
    expect_stat cache_miss 1
    expect_stat files_in_cache 2
    expect_exists secondary/CACHEDIR.TAG
    subdirs=$(find secondary -type d | wc -l)
    if [ "${subdirs}" -ne 1 ]; then # "secondary" itself counts as one
        test_failed "Expected no subdirectories in secondary"
    fi
    expect_file_count 3 '*' secondary # CACHEDIR.TAG + result + manifest

    $CCACHE_COMPILE -c test.c
    expect_stat direct_cache_hit 1
    expect_stat cache_miss 1
    expect_stat files_in_cache 2
    expect_file_count 3 '*' secondary # CACHEDIR.TAG + result + manifest

    $CCACHE -C >/dev/null
    expect_stat files_in_cache 0
    expect_file_count 3 '*' secondary # CACHEDIR.TAG + result + manifest

    $CCACHE_COMPILE -c test.c
    expect_stat direct_cache_hit 2
    expect_stat cache_miss 1
    expect_stat files_in_cache 2 # fetched from secondary
    expect_file_count 3 '*' secondary # CACHEDIR.TAG + result + manifest

    # -------------------------------------------------------------------------
    TEST "Two directories"

    if $HOST_OS_WINDOWS; then
        CCACHE_SECONDARY_STORAGE+=" file:///$(cygpath -m $PWD)/secondary_2"
    else
        CCACHE_SECONDARY_STORAGE+=" file://$PWD/secondary_2"
    fi

    mkdir secondary_2

    $CCACHE_COMPILE -c test.c
    expect_stat direct_cache_hit 0
    expect_stat cache_miss 1
    expect_stat files_in_cache 2
    expect_file_count 3 '*' secondary # CACHEDIR.TAG + result + manifest
    expect_file_count 3 '*' secondary_2 # CACHEDIR.TAG + result + manifest

    $CCACHE -C >/dev/null
    expect_stat files_in_cache 0
    expect_file_count 3 '*' secondary # CACHEDIR.TAG + result + manifest
    expect_file_count 3 '*' secondary_2 # CACHEDIR.TAG + result + manifest

    $CCACHE_COMPILE -c test.c
    expect_stat direct_cache_hit 1
    expect_stat cache_miss 1
    expect_stat files_in_cache 2 # fetched from secondary
    expect_file_count 3 '*' secondary # CACHEDIR.TAG + result + manifest
    expect_file_count 3 '*' secondary_2 # CACHEDIR.TAG + result + manifest

    $CCACHE -C >/dev/null
    expect_stat files_in_cache 0

    rm -r secondary/??
    expect_file_count 1 '*' secondary # CACHEDIR.TAG

    $CCACHE_COMPILE -c test.c
    expect_stat direct_cache_hit 2
    expect_stat cache_miss 1
    expect_stat files_in_cache 2 # fetched from secondary_2
    expect_file_count 1 '*' secondary # CACHEDIR.TAG
    expect_file_count 3 '*' secondary_2 # CACHEDIR.TAG + result + manifest

    # -------------------------------------------------------------------------
    TEST "Read-only"

    $CCACHE_COMPILE -c test.c
    expect_stat direct_cache_hit 0
    expect_stat cache_miss 1
    expect_stat files_in_cache 2
    expect_file_count 3 '*' secondary # CACHEDIR.TAG + result + manifest

    $CCACHE -C >/dev/null
    expect_stat files_in_cache 0
    expect_file_count 3 '*' secondary # CACHEDIR.TAG + result + manifest

    CCACHE_SECONDARY_STORAGE+="|read-only"

    $CCACHE_COMPILE -c test.c
    expect_stat direct_cache_hit 1
    expect_stat cache_miss 1
    expect_stat files_in_cache 2 # fetched from secondary
    expect_file_count 3 '*' secondary # CACHEDIR.TAG + result + manifest

    echo 'int x;' >> test.c
    $CCACHE_COMPILE -c test.c
    expect_stat direct_cache_hit 1
    expect_stat cache_miss 2
    expect_stat files_in_cache 4
    expect_file_count 3 '*' secondary # CACHEDIR.TAG + result + manifest

    # -------------------------------------------------------------------------
if ! $HOST_OS_WINDOWS; then
    TEST "umask"

    CCACHE_SECONDARY_STORAGE="file://$PWD/secondary|umask=022"
    rm -rf secondary
    $CCACHE_COMPILE -c test.c
    expect_perm secondary drwxr-xr-x
    expect_perm secondary/CACHEDIR.TAG -rw-r--r--

    CCACHE_SECONDARY_STORAGE="file://$PWD/secondary|umask=000"
    $CCACHE -C >/dev/null
    rm -rf secondary
    $CCACHE_COMPILE -c test.c
    expect_perm secondary drwxrwxrwx
    expect_perm secondary/CACHEDIR.TAG -rw-rw-rw-
fi
    # -------------------------------------------------------------------------
    TEST "Sharding"

    if $HOST_OS_WINDOWS;then
        CCACHE_SECONDARY_STORAGE=" file:///$(cygpath -m $PWD)/secondary/*|shards=a,b(2)"
    else
        CCACHE_SECONDARY_STORAGE=" file://$PWD/secondary/*|shards=a,b(2)"
    fi

    $CCACHE_COMPILE -c test.c
    expect_stat direct_cache_hit 0
    expect_stat cache_miss 1
    expect_stat files_in_cache 2
    if [ ! -d secondary/a ] && [ ! -d secondary/b ]; then
        test_failed "Expected secondary/a or secondary/b to exist"
    fi

    # -------------------------------------------------------------------------
    TEST "Reshare"

    CCACHE_SECONDARY_STORAGE="" $CCACHE_COMPILE -c test.c
    expect_stat direct_cache_hit 0
    expect_stat cache_miss 1
    expect_stat files_in_cache 2
    expect_stat primary_storage_hit 0
    expect_stat primary_storage_miss 2
    expect_stat secondary_storage_hit 0
    expect_stat secondary_storage_miss 0
    expect_missing secondary

    $CCACHE_COMPILE -c test.c
    expect_stat direct_cache_hit 1
    expect_stat cache_miss 1
    expect_stat primary_storage_hit 2
    expect_stat primary_storage_miss 2
    expect_stat secondary_storage_hit 0
    expect_stat secondary_storage_miss 0
    expect_missing secondary

    CCACHE_RESHARE=1 $CCACHE_COMPILE -c test.c
    expect_stat direct_cache_hit 2
    expect_stat cache_miss 1
    expect_stat primary_storage_hit 4
    expect_stat primary_storage_miss 2
    expect_stat secondary_storage_hit 0
    expect_stat secondary_storage_miss 0
    expect_file_count 3 '*' secondary # CACHEDIR.TAG + result + manifest

    $CCACHE -C >/dev/null

    $CCACHE_COMPILE -c test.c
    expect_stat direct_cache_hit 3
    expect_stat cache_miss 1
    expect_stat primary_storage_hit 4
    expect_stat primary_storage_miss 4
    expect_stat secondary_storage_hit 2
    expect_stat secondary_storage_miss 0
    expect_file_count 3 '*' secondary # CACHEDIR.TAG + result + manifest

    # -------------------------------------------------------------------------
    TEST "Don't share hits"

    $CCACHE_COMPILE -c test.c
    expect_stat direct_cache_hit 0
    expect_stat cache_miss 1
    expect_stat files_in_cache 2
    expect_stat primary_storage_hit 0
    expect_stat primary_storage_miss 2
    expect_stat secondary_storage_hit 0
    expect_stat secondary_storage_miss 2
    expect_file_count 3 '*' secondary # CACHEDIR.TAG + result + manifest

    $CCACHE -C >/dev/null
    expect_stat files_in_cache 0

    CCACHE_SECONDARY_STORAGE+="|share-hits=false"
    $CCACHE_COMPILE -c test.c
    expect_stat direct_cache_hit 1
    expect_stat cache_miss 1
    expect_stat files_in_cache 0
    expect_stat primary_storage_hit 0
    expect_stat primary_storage_miss 4
    expect_stat secondary_storage_hit 2
    expect_stat secondary_storage_miss 2
    expect_file_count 3 '*' secondary # CACHEDIR.TAG + result + manifest


    # -------------------------------------------------------------------------
    TEST "Basedir"

    ## init test ##
    mkdir -p dir1/src dir1/include
    cat <<EOF >dir1/src/test.c
#include <stdarg.h>
#include <test.h>
EOF
    cat <<EOF >dir1/include/test.h
int test;
EOF
    cp -r dir1 dir2
    backdate dir1/include/test.h dir2/include/test.h
    cp -r dir1 dir3
    backdate dir1/include/test.h dir3/include/test.h

    ## test case ##

    cd dir1
    CCACHE_BASEDIR="`pwd`" $CCACHE_COMPILE -I`pwd`/include -c src/test.c
    expect_stat direct_cache_hit 0
    expect_stat cache_miss 1
    expect_stat files_in_cache 2
    expect_stat primary_storage_hit 0
    expect_stat primary_storage_miss 2
    expect_stat secondary_storage_hit 0
    expect_stat secondary_storage_miss 2
    expect_file_count 3 '*' ../secondary # CACHEDIR.TAG + result + manifest

    $CCACHE -C >/dev/null
    expect_stat files_in_cache 0

    cd ../dir2
    CCACHE_BASEDIR="`pwd`" $CCACHE_COMPILE -I`pwd`/include -c src/test.c
    expect_stat direct_cache_hit 1
    expect_stat cache_miss 1
    expect_stat files_in_cache 2
    expect_stat primary_storage_hit 0
    expect_stat primary_storage_miss 4
    expect_stat secondary_storage_hit 2
    expect_stat secondary_storage_miss 2
    expect_file_count 3 '*' ../secondary # CACHEDIR.TAG + result + manifest

    cd ../dir3
    CCACHE_BASEDIR="`pwd`" $CCACHE_COMPILE -I`pwd`/include -c src/test.c
    expect_stat direct_cache_hit 2
    expect_stat cache_miss 1
    expect_stat files_in_cache 2
    expect_stat primary_storage_hit 2
    expect_stat primary_storage_miss 4
    expect_stat secondary_storage_hit 2
    expect_stat secondary_storage_miss 2
    expect_file_count 3 '*' ../secondary # CACHEDIR.TAG + result + manifest


}
