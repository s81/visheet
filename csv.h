#ifndef CSV_H
#define CSV_H

typedef enum { CSV_OK, CSV_ERR_FILE } CsvError;

CsvError csv_load(const char *path);
CsvError csv_save(const char *path);

#endif
