import os
import signal
import subprocess
import time
import multiprocessing
import ctypes
import sys
import re
from enum import Enum
import shutil

CBEIGEBG = '\33[46m'
CEND = '\33[0m'
CBOLD = '\33[1m'
CYELLOW = '\33[33m'
CGREEN = '\33[32m'
CRED = '\33[31m'
CREDBG2 = '\33[101m'
CREDBG = '\33[41m'
CBLUEBG = '\33[44m'
SUCCESS = 0
SKIP = 1
FAIL = 2

FLAGS_PATTERN = re.compile(r"//\s+Flags:(.*)")
TEMP_FOLDER_PATTERN = "common.refreshTmpDir();"

class TimeoutException(Exception):
    pass


def timeout_alarm_handler(signum, frame):
    raise TimeoutException


def divide_list(l, n):
    c = int(round(len(l) / n))
    if c == 0:
        c = 1
    for i in range(0, len(l), c):
        yield l[i:i + c]


class Runner():
  def __init__(self, options):
    self.current_path = os.path.dirname(__file__)
    self.project_path = os.path.join(self.current_path, "../../../../../")
    self.execute_file_path = os.path.join(self.project_path, "node")
    self.test_folder_path = os.path.join(self.project_path, "test")
    self.test_temp_folder_path = os.path.join(self.test_folder_path, "tmp")
    if not os.path.exists(self.execute_file_path):
      raise Exception('There is no "node.js"!')

    self.pool = multiprocessing.Pool(processes=options.process)
    self.print_lock = multiprocessing.Lock()
    self.list_lock = multiprocessing.Lock()
    self.result_lock = multiprocessing.Lock()
    self.queue = multiprocessing.Queue()

    signal.signal(signal.SIGALRM, timeout_alarm_handler)

    self.options = options
    self.skiplist_path = os.path.join(self.current_path, "../skip_list.txt")
    self.skiplist = []
    with open(self.skiplist_path, "r") as f:
      while True:
        line = f.readline()
        if not line:
            break
        path = line.strip()
        self.skiplist.append(path)


    self.sequential_list_path = os.path.join(self.current_path, "../sequential_list.txt")
    self.sequential_list = []
    with open(self.sequential_list_path, "r") as f:
      while True:
        line = f.readline()
        if not line:
            break
        path = line.strip()
        self.sequential_list.append(path)

    self.options.filter = ['test-' + f + '-' for f in self.options.filter]

    self.result = multiprocessing.Array(ctypes.c_uint, 3)
    self.all_fail_list = multiprocessing.Manager().list()
    self.all_test_list = []
    self.all_skip_list = []

  def run(self, test_module_list):
    parallel_list = []
    sequential_list = []
    for test_folder in test_module_list:
      p, s = self._get_module_list(test_folder)
      parallel_list.extend(p)
      sequential_list.extend(s)

    parallel_list.sort()
    sequential_list.sort()
    self._module_test_run(parallel_list, sequential_list)

    self._report()
    if self.result[FAIL]:
        sys.exit(1)

  def _clear_temp_dir(self):
    try:
      shutil.rmtree(self.test_temp_folder_path)
      os.mkdir(self.test_temp_folder_path)
    except:
      print("Cannot make or remove directory")

  def _get_module_list(self, test_folder):
    parallel_list = []
    sequential_list = []
    test_path = os.path.join(self.test_folder_path, test_folder)
    filenames = os.listdir(test_path)
    for filename in filenames:
      if filename.endswith('.js'):
        file_path = os.path.join(test_path, filename)
        file_path = os.path.relpath(file_path)

        # check skip
        if not self.options.all:
          if file_path in self.skiplist:
            self.all_test_list.append(file_path)
            self.all_skip_list.append(file_path)
            continue
        # check filter
        if len(self.options.filter) > 0:
          if not any(file_path.find(f) > 0 for f in self.options.filter):
            self.all_test_list.append(file_path)
            self.all_skip_list.append(file_path)
            continue
        
        self.all_test_list.append(file_path)
        if file_path in self.sequential_list:
          sequential_list.append(file_path)
        else:
          parallel_list.append(file_path)
    return (parallel_list, sequential_list)

  def _module_test_run(self, parallel_list, sequential_list):
    divided_test_list = list(divide_list(parallel_list, self.options.process - 1))

    processes = []
    # parallel test
    for test in divided_test_list:
      p = multiprocessing.Process(
        target=self._exec_test, args=(self, test, False))
      p.start()
      processes.append(p)
    
    # sequential test
    p = multiprocessing.Process(
      target=self._exec_test, args=(self, sequential_list, True))
    p.start()
    processes.append(p)

    for p in processes:
      p.join()

  def _run_subprocess(parent_queue, command):
    process = subprocess.Popen(args=command,
                              cwd=path.TEST_ROOT,
                              stdout=subprocess.PIPE,
                              stderr=subprocess.STDOUT)

    stdout = process.communicate()[0]
    exitcode = process.returncode

    parent_queue.put_nowait([exitcode, stdout])

  def _add_fail_list(self, fail):
    with self.list_lock:
      self._print(fail)
      self.all_fail_list.append(fail)

  def _add_result_count(self, type):
    with self.result_lock:
      self.result[type] += 1

  @staticmethod
  def _exec_test(self, filelist, isSequential=False):
    for file in filelist:
      self._run_node(self, file, isSequential)

  def _print(self, message):
    with self.print_lock:
      print(message)

  @staticmethod
  def _run_node(self, test_file_path, isSequential):
    # search execution Flag
    flag = []
    with open(os.path.join(self.project_path, test_file_path), 'r') as source:
      flags_match = FLAGS_PATTERN.search(source.read())
      if (flags_match):
          flag = flags_match.group(1).strip().split()
    try:
      if isSequential:
        self._clear_temp_dir()
        time.sleep(1)

      signal.alarm(self.options.timeout)
      command = [self.execute_file_path] + flag + [test_file_path]
      p = subprocess.Popen(command,
                            stdin=subprocess.PIPE, shell=False,
                            stdout=subprocess.PIPE, stderr=subprocess.PIPE)
      output = p.communicate()
      exitcode = p.returncode
      if exitcode == 0:
        self._print(CGREEN + "[PASS]: " + test_file_path + CEND)
        self._add_result_count(SUCCESS)
      else:
        self._print(CRED + "[FAIL]: " + test_file_path + CEND)
        self._print(output[0])
        self._print(output[1])
        self._add_fail_list(test_file_path)
        self._add_result_count(FAIL)

    except Exception as e:
      self._print(CRED + "[FAIL]:(TimeOut) " + test_file_path + CEND)
      self._add_fail_list(test_file_path)
      self._add_result_count(FAIL)
      if p == None:
        p.kill()
    finally:
      signal.alarm(0)

  def _report(self):
    print('')
    if self.all_skip_list:
      print(CREDBG2 + CBOLD + '<<Skip Test List>>' + CEND)
      self.all_skip_list.sort()
      for skip in self.all_skip_list:
        print(skip)
      print('')

    if self.all_fail_list:
      print(CREDBG2 + CBOLD + '<<Failed Test List>>' + CEND)
      self.all_fail_list.sort()
      for fail in self.all_fail_list:
          print(fail)
      print('')

    test_file_cnt = int(self.result[SUCCESS] + self.result[FAIL])
    total_cnt = test_file_cnt + len(self.all_skip_list)
    assert total_cnt == len(self.all_test_list)

    persentage = 0
    if total_cnt != 0:
      persentage = float(self.result[SUCCESS]) / float(total_cnt) * 100.0
    print(CBEIGEBG + CBOLD + '<<Test Result>>' + CEND)
    print(' Total: %4d' % (total_cnt))
    print(CGREEN + ' PASS: %4d (%.2f%%)' %
          (self.result[SUCCESS], persentage) + CEND)
    print(CRED + ' FAIL: %4d' % (self.result[FAIL]) + CEND)
    print(CYELLOW + ' SKIP: %4d' % (len(self.all_skip_list)) + CEND)
    print('')
