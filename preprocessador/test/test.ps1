param(
    [string]$AsmFile,
    [string]$ExpectedFile
)

if (-not (Test-Path $AsmFile)) {
    Write-Host "Arquivo ASM não encontrado: $AsmFile" -ForegroundColor Red
    exit 1
}
if (-not (Test-Path $ExpectedFile)) {
    Write-Host "Arquivo Esperado não encontrado: $ExpectedFile" -ForegroundColor Red
    exit 1
}

# Executa o pré-processador
Write-Host "Executando preprocessador..."
..\src\main.exe $AsmFile
if ($LASTEXITCODE -ne 0) {
    Write-Host "Erro na execucao do preprocessador" -ForegroundColor Red
    exit $LASTEXITCODE
}

# Define caminho do arquivo gerado
$PreFile = [System.IO.Path]::ChangeExtension($AsmFile, '.pre')

# Função para ler linhas não vazias e trim de ambas pontas
function Get-ContentTrimmed($path) {
    Get-Content $path | ForEach-Object {
        $line = $_.Trim()
        if ($line.Length -gt 0) { $line }
    }
}

$expected = Get-ContentTrimmed $ExpectedFile
$generated = Get-ContentTrimmed $PreFile

# Compara linha a linha
$max = [Math]::Max($expected.Count, $generated.Count)
$differences = @()
for ($i = 0; $i -lt $max; $i++) {
    $e = if ($i -lt $expected.Count) { $expected[$i] } else { "<MISSING>" }
    $g = if ($i -lt $generated.Count) { $generated[$i] } else { "<MISSING>" }
    if ($e -ne $g) {
        $differences += "Linha $($i+1):`n  Esperado: $e`n  Gerado  : $g`n"
    }
}

if ($differences.Count -eq 0) {
    Write-Host "✅ Teste aprovado: saida identica ao esperado."
    exit 0
} else {
    Write-Host "❌ Teste falhou: diferenças encontradas:`n"
    $differences | ForEach-Object { Write-Host $_ }
    exit 1
}
