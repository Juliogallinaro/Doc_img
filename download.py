#!/usr/bin/env python3
"""
Uso:
  python hotmart_dl.py <url_m3u8_1080p> <nome_saida.mp4>

Exemplo:
  python hotmart_dl.py "https://vod-akm.play.hotmart.com/video/.../hls/...-video=3496646.m3u8?..." "aula01_1080p.mp4"
"""

import subprocess
import sys
from pathlib import Path

HEADERS = (
    "Origin: https://cf-embed.play.hotmart.com\r\n"
    "Referer: https://cf-embed.play.hotmart.com/\r\n"
    "User-Agent: Mozilla/5.0 (Windows NT 10.0; Win64; x64) "
    "AppleWebKit/537.36 (KHTML, like Gecko) Chrome/147.0.0.0 Safari/537.36\r\n"
)

def download(url: str, output: Path):
    cmd = [
        "ffmpeg",
        "-headers", HEADERS,
        "-i", url,
        "-c", "copy",
        "-bsf:a", "aac_adtstoasc",
        "-movflags", "+faststart",
        "-y",
        str(output),
    ]
    print(f"[→] {output.name}")
    result = subprocess.run(cmd)
    if result.returncode == 0:
        mb = output.stat().st_size / 1024 / 1024
        print(f"[✓] {output} ({mb:.1f} MB)")
    else:
        print(f"[✗] Erro {result.returncode}")
        sys.exit(1)

if __name__ == "__main__":
    if len(sys.argv) != 3:
        print(__doc__)
        sys.exit(1)
    download(sys.argv[1], Path(sys.argv[2]))