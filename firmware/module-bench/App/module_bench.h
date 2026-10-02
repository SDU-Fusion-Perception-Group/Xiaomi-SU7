#ifndef MODULE_BENCH_H
#define MODULE_BENCH_H

#ifdef __cplusplus
extern "C" {
#endif

void ModuleBench_Init(void);
void Rs485Task_Entry(void const *argument);
void Can3Task_Entry(void const *argument);
void TelemetryTask_Entry(void const *argument);

#ifdef __cplusplus
}
#endif

#endif
