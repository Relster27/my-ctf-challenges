# Intended exploitation path
- FSOP `_IO_list_all`
- Overwrite `__exit_funcs`

# Vulns
- Type confusion ketika menerima input berat badan baru (`actorWeight`). Vuln func = `change_sub()`
- Type confusion menyebabkan satu byte overflow pada pointer `actorName`
- Type confusion diatas dichain agar menjadi **UAF**

# Step-by-step
### **Leak libc**
1. **Malloc 1 chunk khusus (index 0):** Berfungsi sebagai `vuln chunk` untuk mengontrol pointer `actorName` melalui *Type Confusion*.
2. **Malloc & Free 8 chunk:** Alokasikan 8 chunk, lalu bebaskan (`free`) seluruhnya untuk mengisi *bins*.
    * **State Bins:**
        * `tcachebins`: `size 0x20` (7 chunks) | `size 0x50` (7 chunks)
        * `fastbins`: `size 0x20` (1 chunk) | `size 0x50` (1 chunk)
3. **Trigger Bin Consolidation:** Masukkan input angka dengan panjang karakter > `0x410` (1040) pada menu utama. Internal `scanf()` akan gabungin kedua chunk ini, sehingga chunk pada `fastbins` akan masuk ke dalam `smallbins`.

4. **Lakukan Type confusion**: Pada index-0, lakukan `change` pada member `actorWeight`. Targetnya adalah chunk yang ada di `smallbins` yang sudah kita setup pada step ke-3.
5. **Leak & Parse libc leak**: Pilih opsi ke-4 (`check_sub`) untuk ngeleak address.

### **Leak heap** (lanjut dari step sebelumnya)
1. **Malloc 8 chunk**: Alokasikan lagi delapan chunk, lalu free satu chunk.
2. **Edit chunk index 0**: Edit `actorWeight` untuk ngepoint ke chunk yang barusan difree, specifically ke bagian `actorName`.
3. **Leak & Parse heap leak**: Pilih opsi ke-4 (`check_sub`) untuk ngeleak address.

### **Code execution**
Dengan pattern yang sama lakuin tcache poisoning biar dapat arbitrary write, tinggal pilih aja mau targetin `__exit_funcs` atau `_IO_list_all`

Dari sini cukup straight forward, kedua `Intended exploitation path` sama-sama bisa ditrigger dengan opsi ke-5 (`exit`)