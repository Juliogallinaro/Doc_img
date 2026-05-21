import argparse
import math
from pathlib import Path

from pypdf import PdfReader, PdfWriter


def _rotacao_normalizada(pagina) -> int:
    # Algumas páginas já têm rotação embutida no metadado /Rotate.
    return int(pagina.get("/Rotate", 0)) % 360


def _angulo_dominante_texto(pagina) -> int | None:
    pesos: dict[int, float] = {0: 0.0, 90: 0.0, 180: 0.0, 270: 0.0}

    def _visitante_texto(texto, _cm, tm, _font_dict, font_size):
        txt = (texto or "").strip()
        if not txt:
            return

        # A direção da linha de base vem da matriz de texto (tm[0], tm[1]).
        ang = (math.degrees(math.atan2(float(tm[1]), float(tm[0]))) + 360.0) % 360.0

        # Agrupa no quadrante mais próximo para estabilizar ruído numérico.
        candidatos = [0, 90, 180, 270]
        ang_quadrante = min(candidatos, key=lambda x: min((ang - x) % 360, (x - ang) % 360))

        peso = max(len(txt), 1) * max(float(font_size or 1.0), 1.0)
        pesos[ang_quadrante] += peso

    try:
        pagina.extract_text(visitor_text=_visitante_texto)
    except Exception:
        return None

    angulo, peso = max(pesos.items(), key=lambda item: item[1])
    if peso <= 0:
        return None
    return angulo


def corrigir_para_retrato(entrada: Path, saida: Path) -> tuple[int, int]:
    reader = PdfReader(str(entrada))
    writer = PdfWriter()

    total_paginas = 0
    paginas_corrigidas = 0

    for pagina in reader.pages:
        total_paginas += 1

        largura = float(pagina.mediabox.width)
        altura = float(pagina.mediabox.height)
        rotacao = _rotacao_normalizada(pagina)

        # Dimensões visuais reais, já considerando rotação existente da página.
        if rotacao in (90, 270):
            largura_visual, altura_visual = altura, largura
        else:
            largura_visual, altura_visual = largura, altura

        if largura_visual > altura_visual:
            angulo_texto = _angulo_dominante_texto(pagina)

            # Se o texto estiver "deitado" para a esquerda (270), gira no sentido oposto.
            if angulo_texto == 270:
                pagina.rotate(270)
            else:
                pagina.rotate(90)
            paginas_corrigidas += 1

        writer.add_page(pagina)

    with saida.open("wb") as f:
        writer.write(f)

    return total_paginas, paginas_corrigidas


def main() -> None:
    parser = argparse.ArgumentParser(
        description="Corrige páginas em paisagem para retrato em um PDF."
    )
    parser.add_argument("entrada", type=Path, help="Caminho do PDF de entrada")
    parser.add_argument("saida", type=Path, help="Caminho do PDF de saída")
    args = parser.parse_args()

    if not args.entrada.exists():
        raise FileNotFoundError(f"Arquivo não encontrado: {args.entrada}")

    total, corrigidas = corrigir_para_retrato(args.entrada, args.saida)

    print(f"Total de páginas: {total}")
    print(f"Páginas giradas para retrato: {corrigidas}")
    print(f"PDF corrigido salvo em: {args.saida}")


if __name__ == "__main__":
    main()
