from subprocess import run, STDOUT, PIPE
from BColors import BColors


class TestHandler:
    def __init__(self, prefix):
        self.prefix = prefix
        print(100*"=" + "\nStarting test: " + prefix)

    def proc_run(self, command, arguments=None):
        self.log("Running command: " + command + ((" with arguments: " + str(arguments)) if arguments else ""))
        if isinstance(arguments, list):
            cmd = [command] + arguments
        elif arguments:
            cmd = [command, arguments]
        else:
            cmd = command
        ret = run(cmd, stderr=STDOUT, stdout=PIPE, shell=True)
        split_out = ret.stdout.decode('utf-8').split("\n") if ret.stdout else ""
        tabbed_out = ""
        for line in split_out:
            line = self.prefix + " |    " + line + '\n'
            tabbed_out += line
        self.log("output: \n" + tabbed_out + "\n\treturn code: " + str(ret.returncode))
        return ret.stdout.decode('utf-8'), ret.returncode

    def log(self, msg):
        print(self.prefix + " | " + str(msg) + "\n")

    def print_result(self, result):
        main_msg = 100*"=" + '\n' + self.prefix + " test result: "
        result_msg = ((BColors.OKGREEN + "PASS") if result else (BColors.FAIL + "FAIL")) + BColors.ENDC
        print(main_msg + result_msg)
