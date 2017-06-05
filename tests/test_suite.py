from TestHandler import TestHandler
from sanity_tests import *
from basic_functions_tests import *
import time
from subprocess import Popen


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
    out, _ = test_handler.proc_run("../input.out 'i:15' & ( sleep 10; ../output.out i:15 )")
    end = time.time()
    return '15' in out and end-start >= 10


def double_out_and_in(test_handler):
    test_handler.shmem_cleanup()
    test_handler.proc_run("../output.out 'i:15'")
    test_handler.proc_run("../output.out 'i:20'")
    out, _ = test_handler.proc_run("../input.out 'i:15'")
    if '15' not in out:
        return False
    out, _ = test_handler.proc_run("../input.out 'i:20'")
    return '20' in out


def string_over_max(test_handler):
    out, retcode = test_handler.proc_run('../output.out \'s:\"{}\"\''.format(64*"A"))
    return retcode != 0


def string_with_comma(test_handler):
    funny_str = 'funny string,i:1 right'
    test_handler.proc_run('../output.out \'s:\"{}\"\''.format(funny_str))
    out, _ = test_handler.proc_run("../input.out 's:{}'".format('*'))
    return funny_str in out


def nine_data_tuple(test_handler):
    eight_datas = "i:1,f:2.0,s:\"three\",i:4,f:5.1,s:\"six\",i:7,f:8.8"
    eight_datas_pattern = "i:1,f:>=2.0,s:\"three\",i:4,f:>=5.1,s:\"six\",i:7,f:>=8.8"
    nine_datas = eight_datas + ",s:\"this should be ignored\""
    eight_datas_printed = "1\n2\nthree\n4\n5.1\nsix\n7\n8.8"
    test_handler.proc_run('../output.out \'{}\''.format(nine_datas))
    out, retcode = test_handler.proc_run('../input.out \'{}\''.format(eight_datas_pattern))
    return eight_datas_printed in out


def max_tuples_possible(test_handler):
    test_handler.shmem_cleanup()
    for i in range(256):
        out, retcode = test_handler.proc_run("timeout 5 ../output.out 'i:{}'".format(i))
        if retcode != 0:
            return False
    out, retcode = test_handler.proc_run("timeout 5 ../output.out 'i:{}'".format(256))
    if retcode == 0:
        return False
    out, _ = test_handler.proc_run("../input.out i:50")
    if '50' not in out:
        return False
    out, retcode = test_handler.proc_run("../output.out i:500")
    if retcode != 0:
        return False
    out, _ = test_handler.proc_run("../input.out i:500")
    return '500' in out


def max_queue(test_handler):
    test_handler.shmem_cleanup()
    run_template = "../input.out i:{}"
    run_all = ""
    for i in range(32):
        Popen(run_template.format(i), shell=True)
    test_handler.proc_run(run_all)
    out, retcode = test_handler.proc_run(run_template.format(32))
    if retcode == 0:
        return False
    test_handler.proc_run("../output.out i:15")
    start = time.time()
    out, retcode = test_handler.proc_run("timeout 10 " + run_template.format(32))
    end = time.time()
    return end-start >= 10


def test(title, test_method):
    test_handler = TestHandler(title)
    result = test_method(test_handler)
    test_handler.print_result(result)
    return result


def main():
    # Sanity 1 - create shmem
    test("Sanity #1 - ShMem Create", sanity_create)
    # Sanity 2 - delete shmem
    test("Sanity #2 - ShMem Delete", sanity_delete)
    # Sanity 3 - add a string-only tuple and get it back
    test("Sanity #3 - String tuple retrieve", sanity_string)
    # Sanity 4 - add a int-only tuple and get it back
    test("Sanity #4 - Int tuple retrieve", sanity_int)
    # Sanity 5 - add a float-only tuple and get it back
    test("Sanity #5 - Float tuple retrieve", sanity_float)
    # Empty strings - should be supported, no?
    test("[E01] Empty string", empty_string)
    # Read operation shouldn't remove tuples from shmem
    test("[E02] Double read", double_read)
    # But input should. second call should wait for the new tuple
    test("[E03] Double input", double_input)
    # > and < operators for integers
    test("[E04] Inequality operators for ints", inequality_operators_int)
    # ^ but for floats
    test("[E05] Inequality operators for floats", inequality_operators_float)
    # check if str wildcard (*) works, and if strings at 63 characters are supported
    test("[E06] Long string and wildcard", string_at_max_and_wildcard)
    # wildcard for ints
    test("[E07] Wildcard for integers", int_wildcard)
    # wildcard for floats
    test("[E08] Wildcard for floats", float_wildcard)
    # order should matter - let's check for that too
    test("[E09] Data order in tuples", order_test)
    # string is @ 64 chars, which is right over max - should throw an exception
    test("[E10] String over max limit", string_over_max)
    # string has a comma and a fake integer declaration, should be fine since it's in ""
    test("[E11] String with a comma", string_with_comma)
    # if there are over 9 datas in a tuple you want to add, anything over that should be ignored
    test("[E12] Nine tuple_datas in a pattern", nine_data_tuple)
    # by default you can add 256 tuples, let's see if this works out
    test("[E13] Max tuples possible", max_tuples_possible)
    # queue length max is 20 too, let's check it out
    test("[E14] Max queue possible", max_queue)
    # input on a tuple that went in before shouldn't stop others
    test("[E15] Double output, double input", double_out_and_in)
    # string inequality tests
    test("[E16] Inequality operators for strings", inequality_operators_str)

    TestHandler.summarize_tests()

if __name__ == "__main__":
    main()
