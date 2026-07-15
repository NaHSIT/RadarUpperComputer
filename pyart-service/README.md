# Py-ART 风场反演服务

该模块是上位机的独立算法进程。C++ 通过 `PyArtWindService` 启动 Python worker，交换 UTF-8 JSON，不在 UI 线程内执行科学计算。

## 算法边界

- 使用 `pyart.retrieve.vad_browning` 处理单仰角、多方位径向速度扫描。
- 输入必须至少包含 16 条有效方位射线；五波束快照不能直接满足 VAD 假设。
- CNR 低于阈值、非有限值和被掩膜距离门不参加反演。
- 风向为气象学风向：真北为 0 度，顺时针，表示风的来向。

## 本地运行

```powershell
cd E:\RadarUpperComputer\pyart-service
python -m pip install -e .
python examples\generate_synthetic_request.py
python -m radar_pyart_service.worker --request examples\synthetic_vad_request.json
```

未安装为 Python 包时，可将 `pyart-service/src` 加入 `PYTHONPATH`。C++ 适配器会自动完成这一设置。

## 标准产品

- JSON：`radar.wind-profile/1.0`，作为上位机领域模型交换格式。
- CSV：UTF-8 BOM，明确列名和单位，适合 Excel 与第三方处理。
- NetCDF：CF-1.11 profile，采用 `eastward_wind`、`northward_wind`、`wind_speed`、`wind_from_direction` 标准字段。
- CfRadial：保存输入极坐标径向速度与 CNR，便于复算和审计。
- PNG：1600 × 1000、150 DPI、白底，标注算法、观测时间和有效层数。

## 正式部署

正式工业主板建议 Python 3.11，使用独立虚拟环境，并固定 `arm-pyart` 2.2.x。算法进程异常不影响上位机基础 TCP 通信和已有风场显示。

