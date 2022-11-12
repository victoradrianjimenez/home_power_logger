# coding=utf-8

import csv


class CSVManager:

    def __init__(self, filename: str, columns=None, encoding='UTF8', newline='', append=True):
        # open the file in the write mode
        self.f = open(filename, 'a' if append else 'w', encoding=encoding, newline=newline)

        # create the csv writer
        self.writer = csv.writer(self.f)

        # write the header line
        if columns:
            self.write(columns)

    def write(self, row):
        # write a row to the csv file
        self.writer.writerow(row)

    def close(self):
        self.f.close()

    def __del__(self):
        self.close()
