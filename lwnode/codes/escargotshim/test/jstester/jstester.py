#!/usr/bin/env python

import argparse
from api.runner import Runner

def parse_options():
  parser = argparse.ArgumentParser()
  parser.add_argument('--timeout',
                      metavar='SEC', default=30, type=int,
                      help='specify the timeout (default: %(default)s sec)')
  parser.add_argument('--process',
                      default=7, type=int,
                      help='number of process (default: %(default))')
  parser.add_argument('--filter', '-f',
                      default=[], nargs='+',
                      help='test name(ex: "-f buffer http")')
  parser.add_argument('--module', '-m',
                      default=[], nargs='+',
                      help='test folder name(ex: "-m parallel")')
  parser.add_argument('--all', '-a',
                      default=False, action='store_true',
                      help='test all file')
  return parser.parse_args()

def main():
  options = parse_options()

  test_module_list = ['parallel']
  if len(options.module):
    test_module_list = module

  runner = Runner(options)
  runner.run(test_module_list)

if __name__ == '__main__':
  main()
