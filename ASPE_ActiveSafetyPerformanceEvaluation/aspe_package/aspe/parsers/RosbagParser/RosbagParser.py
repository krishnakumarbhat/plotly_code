import rospy
import tqdm
import yaml
from rosbag.bag import Bag

from aspe.parsers.aptiv_data_parser import DataParser


class RosbagParser(DataParser):
    def __init__(self):
        self.bag_info_dict = None
        self.parsed_data = dict()

    def parse(self, log_file_path: str, f_get_info: bool = False) -> dict:
        self._check_file(log_file_path)

        log_name = log_file_path.split('\\')[-1]
        print(f'Start parsing log: {log_name}')

        with Bag(log_file_path, 'r') as bag:
            self._get_bag_info(bag, f_get_info)

            topics_all = self._get_bag_topics(bag)
            topics = self._drop_image_topics(topics_all)

            with tqdm.tqdm(total=bag.get_message_count(topics)) as pbar:
                self._read_msg(bag, topics, pbar)
        return self.parsed_data

    def _read_msg(self, bag: Bag, topics: list, pbar: tqdm.std.tqdm):
        for topic, msg, t in bag.read_messages(topics):
            pbar.update(1)
            self._put_msg_in_parsed(topic, msg, t)

    def _put_msg_in_parsed(self, topic: str, msg, t: rospy.rostime.Time):
        if self.parsed_data.get(topic, None) is not None:
            self.parsed_data[topic][str(t.to_time())] = msg
        else:
            self.parsed_data[topic] = {str(t.to_time()): msg}

    def _get_bag_info(self, bag: Bag, f_get_info: bool):
        if f_get_info:
            self.bag_info_dict = yaml.safe_load(bag._get_yaml_info())

    @staticmethod
    def _get_bag_topics(bag: Bag) -> list:
        return list(bag.get_type_and_topic_info()[1].keys())

    @staticmethod
    def _drop_image_topics(topics: list) -> list:
        return [topic for topic in topics if 'webcam' not in topic]


if __name__ == '__main__':
    path = r"path/to/rosbag.bag"
    path = r"C:\Users\nj5tl5\Downloads\lincoln_mkz_2023-02-21-13-53-41_1.bag"

    parser = RosbagParser()
    parsed = parser.parse(log_file_path=path,
                          f_get_info=True)
    print('Done')
