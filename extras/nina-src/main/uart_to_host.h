#pragma once

void init_uart();
void read_from_uart(void* out, size_t len);
void write_to_uart(void* data, size_t len);
