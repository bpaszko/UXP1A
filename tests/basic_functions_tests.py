import time


def inequality_operators_int(test_handler):
    test_handler.proc_run("../output.out i:15")
    out, _ = test_handler.proc_run("../read.out 'i:<20'")
    if "15" not in out:
        return False
    start = time.time()
    out, _ = test_handler.proc_run("../read.out 'i:>20' & ( sleep 10; ../output.out i:25 )")
    end = time.time()
    if not ('25' in out and end-start >= 10):
        return False
    test_handler.shmem_cleanup()
    test_handler.proc_run("../output.out i:19; ../output.out i:20; ../output.out i:21")
    out, _ = test_handler.proc_run("../input.out 'i:>=20'")
    if '20' not in out:
        return False
    out, _ = test_handler.proc_run("../input.out 'i:>=20'")
    if '21' not in out:
        return False
    out, _ = test_handler.proc_run("../input.out 'i:<=19'")
    return '19' in out


def inequality_operators_float(test_handler):
    test_handler.proc_run("../output.out f:1.5")
    out, _ = test_handler.proc_run("../read.out 'f:<2.0'")
    if "1.5" not in out:
        return False
    start = time.time()
    out, _ = test_handler.proc_run("../read.out 'f:>2.0' & ( sleep 10; ../output.out f:2.5 )")
    end = time.time()
    if '2.5' not in out or end-start < 10:
        return False
    test_handler.shmem_cleanup()
    test_handler.proc_run("../output.out f:1.9; ../output.out f:2.0; ../output.out f:2.1 ")
    out, _ = test_handler.proc_run("../input.out 'f:>=2.0'")
    if '2' not in out:
        return False
    out, _ = test_handler.proc_run("../input.out 'f:>=2.0'")
    if '2.1' not in out:
        return False
    out, _ = test_handler.proc_run("../input.out 'f:<=1.9'")
    return '1.9' in out


def string_at_max_and_wildcard(test_handler):
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
    out, _ = test_handler.proc_run('../input.out \'f:>=1.5,s:"fifteen",i:15\' &'
                                   ' ( sleep 10; ../output.out \'f:1.5,s:"fifteen",i:15\' ) ')
    end = time.time()
    if end-start < 10 and '1.5 fifteen 15' not in out:
        return False
    out, _ = test_handler.proc_run('../input.out \'f:>=1.5,i:15,s:"fifteen"\'')
    return '1.5\n15\nfifteen' in out


def inequality_operators_str(test_handler):
    test_handler.proc_run("../output.out s:\"aaaaa\"")
    test_handler.proc_run("../output.out s:\"ddddd\"")
    out, _ = test_handler.proc_run("../input.out 's:>\"bbbbb\"'")
    if 'ddddd' not in out:
        return False
    out, _ = test_handler.proc_run("../input.out 's:<\"bbbbb\"'")
    return 'aaaaa' in out
