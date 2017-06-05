from subprocess import run, STDOUT, PIPE
from collections import namedtuple
from BColors import BColors

TestResult = namedtuple("Test_result", ["Title", "result"])
AllResults = []

class TestHandler:
    def __init__(self, prefix):
        self.prefix = prefix
        self.testresult = namedtuple("Test_result", ["title", "result"])
        self.testresult.title = prefix
        print(100*"=" + "\nStarting test: " + prefix + "\n" + 100*"=")

    def proc_run(self, command, arguments=None, new_session=False):
        self.log("Running command: " + command + ((" with arguments: " + str(arguments)) if arguments else ""))
        if isinstance(arguments, list):
            cmd = [command] + arguments
        elif arguments:
            cmd = [command, arguments]
        else:
            cmd = command
        ret = run(cmd, start_new_session=new_session, stderr=STDOUT, stdout=PIPE, shell=True)
        split_out = ret.stdout.decode('utf-8').split("\n") if ret.stdout else ""
        tabbed_out = ""
        for line in split_out:
            line = self.prefix + " |   " + line + '\n'
            tabbed_out += line
        self.log("output: \n" + tabbed_out)
        self.log("return code: " + str(ret.returncode))
        return ret.stdout.decode('utf-8'), ret.returncode

    def log(self, msg):
        print(self.prefix + " | " + str(msg) + "\n")

    def print_result(self, result):
        main_msg = 100*"=" + '\n' + self.prefix + " test result: "
        result_msg = ((BColors.OKGREEN + "PASS") if result else (BColors.FAIL + "FAIL")) + BColors.ENDC
        print(main_msg + result_msg)
        self.testresult.result = result_msg
        AllResults.append(self.testresult)

    def shmem_cleanup(self):
        self.log("Cleaning shared memory ...")
        self.proc_run("../read.out -d; ../read.out -c")  # clean up and start anew
        self.log(110*"=" + "\nReal test begins now")

    @staticmethod
    def summarize_tests():
        border = 100*'='
        summary_title = 40*'=' + " TEST SUITE SUMMARY " + 40*'='
        summary_end = 78*'=' + " SUITE RESULT: {:4}" + "  ="
        test_res_template = "||{:<88}|{:^8}|"
        title = test_res_template.format(" Test Name", "Result")
        test_res_template = "||{:<88}|{:^17}|"
        print(3*"\n")
        print(border)
        print(summary_title)
        print(border)
        print(title)
        print(border)
        total_result = True
        for test in AllResults:
            if 'FAIL' in test.result:
                total_result = False
            print(test_res_template.format(test.title, test.result))
        result_msg = ((BColors.OKGREEN + "PASS") if total_result else (BColors.FAIL + "FAIL")) + BColors.ENDC
        print(border)
        print(summary_end.format(result_msg))
        print(border)
