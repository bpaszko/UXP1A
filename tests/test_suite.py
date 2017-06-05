from TestHandler import TestHandler
from sanity_tests import *
import time


def empty_string(test_handler):
    test_handler.proc_run("../output.out 's:\"\"'")
    out, errcode = test_handler.proc_run("timeout 10 ../input.out 's:\"\"'")
    return errcode == 0


def double_read(test_handler):
    test_handler.proc_run("../output.out 'i:15'")
    out, _ = test_handler.proc_run("../read.out 'i:15'")
    if '15' not in out:
        return False
    out, errcode = test_handler.proc_run("timeout 10 ../read.out 'i:15'")
    return errcode == 0


def double_input(test_handler):
    test_handler.proc_run("../output.out 'i:15'")
    out, _ = test_handler.proc_run("../input.out 'i:15'")
    if '15' not in out:
        return False
    start = time.time()
    out, errcode = test_handler.proc_run("../input.out 'i:15' & ( sleep 10; ../output.out i:15 )")
    end = time.time()
    return '15' in out and end-start >= 10


def inequality_operators_int(test_handler):
    test_handler.proc_run("../output.out i:15")
    out, _ = test_handler.proc_run("../read.out 'i:<20'")
    if "15" not in out:
        return False
    start = time.time()
    out, _ = test_handler.proc_run("../read.out 'i:>20' & ( sleep 10; ../output.out i:25 )")
    end = time.time()
    return '25' in out and end-start >= 10


def inequality_operators_float(test_handler):
    test_handler.proc_run("../output.out f:1.5")
    out, _ = test_handler.proc_run("../read.out 'f:<2.0'")
    if "1.5" not in out:
        return False
    start = time.time()
    out, _ = test_handler.proc_run("../read.out 'f:>2.0' & ( sleep 10; ../output.out f:2.5 )")
    end = time.time()
    return '2.5' in out and end-start >= 10


# def greater_or_smaller_or_equal_int(test_handler):
#     shmem_cleanup(test_handler)
#     test_handler.proc_run("../output.out i:21; ../output.out i:20; ../output.out i:19")
#     out, _ = test_handler.proc_run("../read.out 'i:<=20'")
#     if "20" not in out:
#         return False
#     out, _ = test_handler.proc_run("../read.out 'i:==19'")
#     if "19" not in out:
#         return False
#     out, _ = test_handler.proc_run("../read.out 'i:<20'")
#     if "19" not in out:
#         return False
#     out, _ = test_handler.proc_run("../input.out 'i:>=20'")
#     if "21" not in out:
#         return False
#     out, _ = test_handler.proc_run("../input.out 'i:>=20'")
#     return "20" in out
#
#
# def greater_or_smaller_or_equal_float(test_handler):
#     shmem_cleanup(test_handler)
#     test_handler.proc_run("../output.out f:2.1; ../output.out f:2.0; ../output.out f:1.9")
#     out, _ = test_handler.proc_run("../read.out 'f:<=2.0'")
#     if "2.0" not in out:
#         return False
#     out, _ = test_handler.proc_run("../read.out 'f:<2.0'")
#     if "1.9" not in out:
#         return False
#     out, _ = test_handler.proc_run("../read.out 'f:>=2.0'")
#     if "2.1" not in out:
#         return False
#     out, _ = test_handler.proc_run("../input.out 'f:==2.0'")
#     return "2.0" in out


def string_at_max_and_wildcard(test_handler):
    shmem_cleanup(test_handler)
    long_str = 'Aaaaaaaa'\
               'Baaaaaaa'\
               'Caaaaaaa' \
               'Daaaaaaa' \
               'Eaaaaaaa' \
               'Faaaaaaa' \
               'Gaaaaaaa'\
               'Haaaaaa'
    test_handler.proc_run('../output.out \'s:\"{}\"\''.format(long_str))
    out, _ = test_handler.proc_run("../input.out 's:*'")
    return long_str in out


def int_wildcard(test_handler):
    test_handler.proc_run('../output.out "i:15"')
    out, _ = test_handler.proc_run('../input.out "i:*"')
    return '15' in out


def float_wildcard(test_handler):
    test_handler.proc_run('../output.out "f:1.5"')
    out, _ = test_handler.proc_run('../input.out "f:*"')
    return '1.5' in out


def order_test(test_handler):
    test_handler.proc_run('../output.out \'f:1.5,i:15,s:"fifteen"\'')
    start = time.time()
    out, _ = test_handler.proc_run('../input.out \'f:1.5,s:"fifteen",i:15\' &'
                                   ' ( sleep 10; ../output.out \'f:1.5,s:"fifteen",i:15\' ) ')
    end = time.time()
    if end-start < 10 and '1.5 fifteen 15' not in out:
        return False
    out, _ = test_handler.proc_run('../input.out \'f:1.5,i:15,s:"fifteen"\'')
    return '1.5\n15\nfifteen' in out


def shmem_cleanup(test_handler):
    test_handler.log("Cleaning shared memory ...")
    test_handler.proc_run("../read.out -d; ../read.out -c")  # clean up and start anew
    test_handler.log(110*"=" + "\nReal test begins now")


def test(title, test_method):
    test_handler = TestHandler(title)
    result = test_method(test_handler)
    test_handler.print_result(result)
    return result


def main():
    total_res = True
    # Sanity 1 - create shmem
    total_res &= test("Sanity #1 - ShMem Create", sanity_create)
    # Sanity 2 - delete shmem
    total_res &= test("Sanity #2 - ShMem Delete", sanity_delete)
    # Sanity 3 - add a string-only tuple and get it back
    total_res &= test("Sanity #3 - String tuple retrieve", sanity_string)
    # Sanity 4 - add a int-only tuple and get it back
    total_res &= test("Sanity #4 - Int tuple retrieve", sanity_int)
    # Sanity 5 - add a float-only tuple and get it back
    total_res &= test("Sanity #5 - Float tuple retrieve", sanity_float)
    # Empty strings - should be supported, no?
    total_res &= test("Empty string", empty_string)
    # Read operation shouldn't remove tuples from shmem
    total_res &= test("Double read", double_read)
    # But input should. second call should wait for the new tuple
    total_res &= test("Double input", double_input)
    # > and < operators for integers
    total_res &= test("Inequality operators for ints", inequality_operators_int)
    # ^ but for floats
    total_res &= test("Inequality operators for floats", inequality_operators_float)
    # # check out <= and >= for ints - actually not supported
    # total_res &= test("[In]equality operators for ints", greater_or_smaller_or_equal_int)
    # # ^ for floats
    # total_res &= test("[In]equality operators for floats", greater_or_smaller_or_equal_float)

    # check if str wildcard (*) works, and if strings are truncated below 64char
    total_res &= test("Long string and wildcard", string_at_max_and_wildcard)
    # wildcard for ints
    total_res &= test("Wildcard for integers", int_wildcard)
    # wildcard for floats
    total_res &= test("Wildcard for floats", float_wildcard)
    # order should matter - let's check for that too
    total_res &= test("Data order in tuples", order_test)

if __name__ == "__main__":
    main()
