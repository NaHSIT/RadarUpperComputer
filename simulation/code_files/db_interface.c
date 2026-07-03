/**
 * @file db_interface.c
 * @brief 数据库接口实现 - 从SQLite读取仿真数据
 */

#include "db_interface.h"
#include <stdio.h>
#include <string.h>
#include <sqlite3.h>

static sqlite3 *db = NULL;

int db_interface_init(const char *db_path) {
    if (sqlite3_open(db_path, &db) != SQLITE_OK) {
        fprintf(stderr, "数据库打开失败: %s\n", db_path);
        return -1;
    }
    printf("数据库已连接: %s\n", db_path);
    return 0;
}

void db_interface_close(void) {
    if (db) {
        sqlite3_close(db);
        db = NULL;
    }
}

int db_read_wind_field(WindFieldData *wind_field) {
    if (!db || !wind_field) return -1;

    const char *sql =
        "SELECT beam_index, range_gate, wind_speed, wind_direction "
        "FROM wind_field "
        "WHERE id IN (SELECT MAX(id) FROM wind_field GROUP BY beam_index, range_gate) "
        "ORDER BY beam_index, range_gate";

    sqlite3_stmt *stmt;
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, NULL) != SQLITE_OK)
        return -1;

    memset(wind_field, 0, sizeof(WindFieldData));

    while (sqlite3_step(stmt) == SQLITE_ROW) {
        int beam = sqlite3_column_int(stmt, 0);
        int gate = sqlite3_column_int(stmt, 1);

        if (beam < NUM_BEAMS && gate < NUM_RANGE_GATES) {
            wind_field->wind_speed[beam][gate] = (float)sqlite3_column_double(stmt, 2);
            wind_field->wind_direction[beam][gate] = (float)sqlite3_column_double(stmt, 3);
            wind_field->confidence[beam][gate] = 0.9f;
        }
    }

    wind_field->timestamp = 0;
    sqlite3_finalize(stmt);
    return 0;
}

int db_read_system_status(SystemStatus *status) {
    if (!db || !status) return -1;

    const char *sql = "SELECT state, temperature, voltage FROM system_status ORDER BY id DESC LIMIT 1";
    sqlite3_stmt *stmt;

    if (sqlite3_prepare_v2(db, sql, -1, &stmt, NULL) != SQLITE_OK)
        return -1;

    if (sqlite3_step(stmt) == SQLITE_ROW) {
        status->state = SYSTEM_RUNNING;
        status->temperature = (float)sqlite3_column_double(stmt, 1);
        status->voltage = (float)sqlite3_column_double(stmt, 2);
        status->adc_status = true;
        status->ddr_status = true;
        status->clock_status = true;
    }

    sqlite3_finalize(stmt);
    return 0;
}
