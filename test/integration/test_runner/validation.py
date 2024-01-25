from abc import abstractmethod, ABC
from typing import Tuple, Any, List

from common import TestResult, TestSetResult


class TestExecutionValidator(ABC):

    @property
    def name(self):
        return type(self).__name__

    def should_validate(self, name: str, viewed: bool) -> bool:
        return True

    @abstractmethod
    def validate(self, test_data: Any, appview_messages: List[str]) -> TestResult:
        pass


class TestSetValidator(ABC):

    @property
    def name(self):
        return type(self).__name__

    def should_validate(self, name: str) -> bool:
        return True

    @abstractmethod
    def validate(self, result: TestSetResult) -> TestResult:
        pass


def passed(): return TestResult(passed=True)


def failed(err): return TestResult(passed=False, error=err)


class NoFailuresValidator(TestSetValidator):

    def validate(self, result: TestSetResult) -> TestResult:
        if not result.unviewd_execution_data.result.passed:
            return failed(result.unviewd_execution_data.result.error)

        if not result.viewed_execution_data.result.passed:
            return failed(result.viewed_execution_data.result.error)

        return passed()


class GotTheDataFromAppViewValidator(TestSetValidator):

    def validate(self, result: TestSetResult) -> TestResult:
        unviewd_msg_count = len(result.unviewd_execution_data.appview_messages)
        viewed_msg_count = len(result.viewed_execution_data.appview_messages)

        if unviewd_msg_count != 0:
            return failed(f"Expected to have 0 messages but got {unviewd_msg_count}")

        if viewed_msg_count == 0:
            return failed(f"No messages from appview detected")

        return passed()


def validate_all(*assertions: Tuple[bool, str]) -> TestResult:
    for assertions in assertions:
        if not assertions[0]:
            return TestResult(passed=False, error=assertions[1])

    return TestResult(passed=True)


default_test_set_validators = [NoFailuresValidator(), GotTheDataFromAppViewValidator()]
