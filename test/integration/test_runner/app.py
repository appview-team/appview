import argparse
import logging
import sys
from datetime import datetime

import cribl
import elastic
import kafka_test
import splunk
import syscalls
from reporting import print_summary, store_results_to_file
from runner import Runner
from appview import AppViewDataCollector, create_listener
from validation import default_test_set_validators
from watcher import TestWatcher


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("-v", "--verbose", help="increase output verbosity", action="store_true")
    parser.add_argument("-t", "--target", help="target application to test",
                        choices=["splunk", "cribl", "syscalls", "elastic", "kafka"],
                        required=True)
    parser.add_argument("-id", "--execution_id", help="execution id")
    parser.add_argument("-l", "--logs_path", help="path to store the execution results and logs", default="/tmp/")
    parser.add_argument("-s", "--appview_path", help="path to appview application executable",
                        default="/usr/lib/libappview.so")
    parser.add_argument("-m", "--metric_type", help="metric destination type",
                        choices=["udp", "tcp"],
                        default="udp")

    parser.add_argument("--syscalls_tests_config", help="path to syscalls tests config")

    args = parser.parse_args()

    execution_id = args.execution_id or generate_execution_id(args.target)
    logs_path = args.logs_path

    log_level = (logging.INFO, logging.DEBUG)[args.verbose]

    format = "%(asctime)s %(levelname)s %(name)s - %(message)s"
    logging.basicConfig(level=log_level, format=format)

    logging.info(f"Execution ID: {execution_id}")
    logging.info(f"AppView path: {args.appview_path}")
    logging.info(f"Logs path: {logs_path}")
    logging.info(f"Metrics destination type: {args.metric_type}")

    test_watcher = TestWatcher(execution_id)
    test_watcher.start()
    appview_data_collector = AppViewDataCollector(args.metric_type)
    data_listener = create_listener(args.metric_type, appview_data_collector)

    with data_listener:
        try:
            runner = Runner(test_watcher, appview_data_collector)
            runner.add_test_set_validators(default_test_set_validators)
            if args.target == 'splunk':
                splunk.configure(runner, args)
            if args.target == 'cribl':
                cribl.configure(runner, args)
            if args.target == 'syscalls':
                syscalls.configure(runner, args)
            if args.target == 'elastic':
                elastic.configure(runner, args)
            if args.target == 'kafka':
                kafka_test.configure(runner, args)
            runner.run()
            test_watcher.finish()
        except Exception as e:
            logging.error("Tests execution finished with exception: " + str(e), exc_info=True)
            test_watcher.finish_with_error(str(e))

    print_summary(test_watcher.get_all_results())
    store_results_to_file(watcher=test_watcher, path=logs_path, appview_version="")
    logging.shutdown()

    return (0, 1)[test_watcher.has_failures()]


def generate_execution_id(target):
    return f"{target}_{datetime.now().isoformat()}".replace(":", "_")


if __name__ == '__main__':
    sys.exit(main())
