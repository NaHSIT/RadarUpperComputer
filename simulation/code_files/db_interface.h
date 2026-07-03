/**
 * @file db_interface.h
 * @brief 数据库接口 - 从SQLite读取仿真数据
 */

#ifndef DB_INTERFACE_H
#define DB_INTERFACE_H

#include "main.h"

/**
 * @brief 初始化数据库连接
 * @param db_path 数据库文件路径
 * @return 0成功，-1失败
 */
int db_interface_init(const char *db_path);

/**
 * @brief 关闭数据库连接
 */
void db_interface_close(void);

/**
 * @brief 从数据库读取风场数据
 * @param wind_field 输出的风场数据
 * @return 0成功，-1失败
 */
int db_read_wind_field(WindFieldData *wind_field);

/**
 * @brief 从数据库读取系统状态
 * @param status 输出的系统状态
 * @return 0成功，-1失败
 */
int db_read_system_status(SystemStatus *status);

#endif
