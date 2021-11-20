[string]$Compiler = $null;
if(Get-Command 'clang' -ErrorAction SilentlyContinue) { $Compiler = 'clang'; }
elseif(Get-Command 'gcc' -ErrorAction SilentlyContinue) { $Compiler = 'gcc'; }
elseif(Get-Command 'tcc' -ErrorAction SilentlyContinue) { $Compiler = 'tcc'; }
else { $Compiler = Read-Host -Prompt 'Please specify C compiler'; }

& $Compiler src/*.c -std=gnu11 -Wl,--gc-sections -pthread -lmbedcrypto -lmbedtls -lmbedx509 -o server.exe
