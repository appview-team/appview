from abc import ABC, abstractmethod
from typing import Any, Tuple, List


class TestResult:
    passed: bool
    error: str

    def __init__(self, passed, error=None):
        self.error = error
        self.passed = passed


class TestExecutionData:
    result: TestResult
    duration: int
    appview_messages: List[str]
    test_data: Any

    def __init__(self):
        self.appview_messages = []
        self.duration = 0
        self.test_data = None


class TestSetResult:
    error: None
    unviewd_execution_data: TestExecutionData
    viewed_execution_data: TestExecutionData
    passed: bool

    def __init__(self):
        self.passed = False
        self.viewed_execution_data = TestExecutionData()
        self.unviewd_execution_data = TestExecutionData()
        self.error = None


class AppController(ABC):

    def __init__(self, name):
        self.__name = name

    @abstractmethod
    def start(self, viewed):
        pass

    @abstractmethod
    def stop(self):
        pass

    @abstractmethod
    def assert_running(self):
        pass

    @property
    def name(self):
        return self.__name


class Test(ABC):

    @abstractmethod
    def run(self, viewed) -> Tuple[TestResult, Any]:
        pass

    @property
    @abstractmethod
    def name(self):
        return None


class ApplicationTest(Test, ABC):

    def __init__(self, app_controller: AppController):
        self.app_controller = app_controller

    def run(self, viewed) -> Tuple[TestResult, Any]:

        self.app_controller.start(viewed)

        try:
            result, data = self.do_run(viewed)
            self.app_controller.assert_running()
        finally:
            self.app_controller.stop()

        return result, data

    @abstractmethod
    def do_run(self, viewed) -> Tuple[TestResult, Any]:
        pass
