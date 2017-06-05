from TestHandler import TestHandler


def main():
    # Sanity 1 - create shmem
    test_handler = TestHandler("Sanity #1 - ShMem Create")
    test_handler.proc_run("../input.out -c")
    out, _ = test_handler.proc_run("ls /dev/shm | grep tuple_space")
    result = "tuple_space" in out
    test_handler.print_result(result)
    # Sanity 2 - delete shmem
    test_handler = TestHandler("Sanity #2 - ShMem Delete")
    test_handler.proc_run("../input.out -d")
    out, _ = test_handler.proc_run("ls /dev/shm | grep tuple_space")
    result = "tuple_space" not in out
    test_handler.print_result(result)
    # Sanity 3 - add a string-only tuple and get it back
    test_handler = TestHandler("Sanity #3 - String tuple retrieve")
    test_handler.proc_run("../output.out -c 's:\"abcd\"'")
    out, _ = test_handler.proc_run("../input.out 's:\"abcd\"'")
    result = 'abcd' in out
    test_handler.print_result(result)
    # Sanity 4 - add a int-only tuple and get it back
    test_handler = TestHandler("Sanity #4 - Int tuple retrieve")
    test_handler.proc_run("../output.out 'i:15'")
    out, _ = test_handler.proc_run("../input.out 'i:15'")
    result = '15' in out
    test_handler.print_result(result)
    # Sanity 5 - add a float-only tuple and get it back
    test_handler = TestHandler("Sanity #5 - Int tuple retrieve")
    test_handler.proc_run("../output.out 'f:1.5'")
    out, _ = test_handler.proc_run("../input.out 'f:1.5'")
    result = '1.5' in out
    test_handler.print_result(result)

if __name__ == "__main__":
    main()