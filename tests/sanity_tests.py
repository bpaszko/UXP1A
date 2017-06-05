def sanity_create(test_handler):
    test_handler.proc_run("../input.out -c")
    out, _ = test_handler.proc_run("ls /dev/shm | grep tuple_space")
    return "tuple_space" in out


def sanity_delete(test_handler):
    test_handler.proc_run("../input.out -d")
    out, _ = test_handler.proc_run("ls /dev/shm | grep tuple_space")
    return "tuple_space" not in out


def sanity_string(test_handler):
    test_handler.proc_run("../output.out -c 's:\"abcd\"'")
    out, _ = test_handler.proc_run("../input.out 's:\"abcd\"'")
    return 'abcd' in out


def sanity_int(test_handler):
    test_handler.proc_run("../output.out 'i:15'")
    out, _ = test_handler.proc_run("../input.out 'i:15'")
    return '15' in out


def sanity_float(test_handler):
    test_handler.proc_run("../output.out 'f:1.5'")
    out, _ = test_handler.proc_run("../input.out 'f:>=1.5'")
    return '1.5' in out
