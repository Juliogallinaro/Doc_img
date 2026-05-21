#!/usr/bin/env python3
"""
Script para gerar thumbnails de imagens
Uso: python gerar_thumbnail.py <caminho_da_imagem> [tamanho]
Exemplo: python gerar_thumbnail.py chip_capilar/fotos/20260505_234414.jpg 250
"""

import sys
from pathlib import Path
from PIL import Image

def gerar_thumbnail(caminho_imagem, tamanho=250):
    """Gera um thumbnail da imagem"""
    try:
        # Converte para Path para melhor compatibilidade com caminhos especiais
        img_path = Path(caminho_imagem)
        
        if not img_path.exists():
            print(f"Erro: Arquivo não encontrado: {img_path}")
            return False
        
        # Abre a imagem
        img = Image.open(img_path)
        
        # Gera nome do thumbnail
        nome_base = img_path.stem
        extensao = img_path.suffix
        thumbnail_path = img_path.parent / f"{nome_base}_thumb{extensao}"
        
        # Redimensiona
        img.thumbnail((tamanho, tamanho), Image.Resampling.LANCZOS)
        img.save(thumbnail_path)
        
        print(f"✓ Thumbnail criado: {thumbnail_path}")
        print(f"  Tamanho: {tamanho}px")
        return True
        
    except Exception as e:
        print(f"Erro: {e}")
        return False

if __name__ == "__main__":
    if len(sys.argv) < 2:
        print("Uso: python gerar_thumbnail.py <caminho_da_imagem> [tamanho=250]")
        print("\nExemplos:")
        print("  python gerar_thumbnail.py chip_capilar/fotos/20260505_234414.jpg")
        print("  python gerar_thumbnail.py chip_capilar/fotos/20260505_234414.jpg 300")
        sys.exit(1)
    
    caminho = sys.argv[1]
    tamanho = int(sys.argv[2]) if len(sys.argv) > 2 else 250
    
    gerar_thumbnail(caminho, tamanho)
