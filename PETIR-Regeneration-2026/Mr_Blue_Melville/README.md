# Vulns
- Arbitrary file open via opsi `open`
- Buffer overflow via opsi `write`
- Ada internal function yang ngeleak heap address via fungsi `history()`
- Leak address di heap chunk via opsi `console`

# Step-by-step
1. Leak binary base (**PIE**) via `open("/proc/self/maps", O_RDONLY);`
2. Setup chunk dengan ngefree heap chunk agar libc address ada di chunk tersebut, pilih opsi `N` pada `write`
3. Lakuin buffer overflow pada opsi `write` dengan return address ngepoint ke `history()` dan chain kembali ke fungsi `main()`
4. Parse heap leak yang dioutputkan fungsi `history()`
5. Leak libc dengan opsi tersembunyi `console`, lalu resolve rop gadgets yang diperlukan
6. Terakhir, lakuin ret2libc attack (seperti di step 3)