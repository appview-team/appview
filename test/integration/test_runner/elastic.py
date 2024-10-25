import logging
from logging import DEBUG
from typing import Tuple, Any

from elasticsearch import Elasticsearch, helpers
from retrying import retry

from common import ApplicationTest, TestResult
from proc import SubprocessAppController
from runner import Runner
from utils import dotdict, random_string
from validation import validate_all

config = dotdict({
    "scheme": "http",
    "host": "localhost",
    "port": 9200
})


class ElasticIndexAndSearchTest(ApplicationTest):

    def do_run(self, viewed) -> Tuple[TestResult, Any]:
        logging.info(f"Connecting to Elasticsearch at {config.scheme} {config.host} {config.port}")
        es = self.__connect_to_es()

        logging.info(f"Elastic info: {es.info()}")

        documents_num = 100000
        logging.info(f"Sending {documents_num} documents to Elastic.")

        index = f"test_{random_string(4)}"
        es.indices.create(
            index=index,
            body={
                "mappings": {
                    "properties": {
                        "bucket": {"type": "integer"},
                        "body": {"type": "keyword"},
                    }
                }
            }
        )

        helpers.bulk(es, self.documents_generator(documents_num), index=index)

        logging.info("Refreshing elastic index")
        es.indices.refresh(index=index)

        docs_in_index = es.count(index=index)["count"]

        limit = 2
        expected_results = int(documents_num / 10)
        sr = es.search(
            index=index,
            body={
                "query": {
                    "term": {"bucket": 2}
                },
                "sort": [{"body": {"order": "desc"}}],
                "size": limit,
            }
        )

        return validate_all(
            (docs_in_index == documents_num,
             f"Expected to have {documents_num} docs in index, but found {docs_in_index}"),
            (sr['hits']['total']['value'] == expected_results,
             f"Expected to have {expected_results} search results, but found {sr['hits']['total']['value']}"),
            (len(sr['hits']['hits']) == limit,
             f"Expected to have {limit} hits returned, but found {len(sr['hits']['hits'])}"),
        ), sr

    @retry(stop_max_attempt_number=5, wait_fixed=5000)
    def __connect_to_es(self):
        logging.info("Attempt")
        elasticsearch = Elasticsearch([{"scheme": config.scheme, "host": config.host, "port": config.port}])
        assert elasticsearch.ping()
        return elasticsearch

    @staticmethod
    def documents_generator(documents_num):
        for i in range(documents_num):
            yield {
                "bucket": i % 10,
                "body": random_string(15)
            }

    @property
    def name(self):
        return "index and search test"


def configure(runner: Runner, config):
    app_controller = SubprocessAppController(["/usr/local/bin/docker-entrypoint.sh"], "elastic", config.appview_path,
                                             config.logs_path, start_wait=60)

    runner.add_tests([ElasticIndexAndSearchTest(app_controller)])


if __name__ == '__main__':
    logging.basicConfig(level=DEBUG)
    es = Elasticsearch([{"scheme": config.scheme, "host": config.host, "port": config.port}])

    index = f"test_{random_string(4)}"

    es.indices.create(
        index=index,
        body={
            "mappings": {
                "properties": {
                    "bucket": {"type": "integer"},
                    "body": {"type": "keyword"},
                }
            }
        }
    )

    helpers.bulk(es, ElasticIndexAndSearchTest.documents_generator(100), index=index)

    es.search(
        index=index,
        body={
            "query": {
                "term": {"user": "kimchy"}
            },
            "size": 2,
        }
    )

    es.search(
        index=index,
        body={
            "size": 8,
        }
    )
