from IPS.DataCollect.icollectdata import IDataCollect


class ParserContext:
    def __init__(self, data_collect_strategy: IDataCollect):
        self.parse_strategy = data_collect_strategy

    def execute(self, infile, outfile):
        self.parse_strategy.collect_data(infile, outfile)
