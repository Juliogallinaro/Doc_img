---
title: Doutorado – Plataforma Microfluídica
markmap:
  colorFreezeLevel: 2
---

## Visão Geral <!-- markmap: fold -->

- Aplicações: geração de esferoides, testes de citotoxicidade, análise óptica e automação CNC
- Abordagem multidisciplinar: microfluídica • impressão 3D • PDMS • CNC • óptica • biologia celular

## Hardware <!-- markmap: fold -->

### Bombas de Seringa (Arduino)   
- Baseadas em **Arduino Nano / Uno**
- Driver: TB6600
- Ensaios:
  - Calibração volumétrica
  - Curvas de vazão (10–500 µL/min)
  - Integração com chips microfluídico

### CNC para Microfluídica
- CNC 3018 / customizada
- Material: Acrílico (PMMA)
- Fresamento de microcanais

## Microfluídica <!-- markmap: fold -->

### Técnicas de Fabricação

- **Impressão 3D direta**
  - Elegoo Saturn 4 Ultra 16k
  - Resinas:
    - High Clear
    - ABS-like
    - Biocompatíveis:
      - priZma Bio Splint: 
        - [Artigo biocompatibilidade](https://drive.google.com/file/d/1C7yh1aE9JgiG2JWLa9L-gHY_POc0hEjP/view?usp=sharing)
  - Parâmetros calibrados (1.4–2.1 s)
  - Problemas: fechamento de canais • envergamento • cura interna
  - [Artigos](https://drive.google.com/drive/folders/10k25BLZkn9ftjy6EAHRU1O4rbI8oY3T0?usp=drive_link)
  


- **Impressão 3D → Moldes em PDMS**   
  - Modelos no Onshape
  - Pós-cura e acabamento superficial
  - Replicação em PDMS (Sylgard 184)
  - Tratamento para não aderência (silanização)

- **Usinagem CNC**
  - Fresamento de microcanais
      - PMMA:
        - [Artigos](https://drive.google.com/drive/folders/1UGEaE2wZF2gBufoUUlp4kxt1sr9EYxjb?usp=drive_link)
      - [Usinagem](https://drive.google.com/drive/folders/1rQCA3WnkzGg6dWyQxfqeuspD6xnW_hT6?usp=drive_link)
  - Brocas de 0.3 mm – força de corte – vibração
  - Comparação com impressão 3D

- **Corte/Laser IPEN**
  - Fabricação híbrida Vidro + PDMS

### Tipos de Chips Desenvolvidos

- Droplet generator (T-junction / flow-focusing)
- Chips para montagem de esferoides
- Canais para citotoxicidade
- Chips multimateriais (resina + PDMS + acrílico): <!-- markmap: fold -->
  - <img src="https://raw.githubusercontent.com/Juliogallinaro/Doc_img/refs/heads/main/chip%20cnc.drawio.png" width="100
  " alt="Funcional"> 
- Testes de transmição óptica

## Ensaios Biológicos <!-- markmap: fold -->

### Citotoxicidade dos Materiais   
- Materiais avaliados:
  - Resina High Clear
  - PDMS
  - Acrílico usinado
  - Chips híbridos
- Ensaios:
  - MTT
  - Live/Dead
  - Tempo de exposição × viabilidade
  - Pré-tratamentos químicos e limpeza

### Cultura de Esferoides
- Tipos celulares: Vero / fibroblastos / outros
- Protocolos:
  - Formação em microchips
  - Ancoragem das esferas
  - Manutenção do fluxo contínuo
- Análises:
  - Imagens ópticas
  - Crescimento temporal
  - Estabilidade no chip

### Testes de Substâncias em Esferoides   
- Drogas / agentes citotóxicos
- Gradiente de concentração (via bombas de seringa)
- Leitura óptica:
  - Absorbância
  - Fluorescência
- Modelos matemáticos:
  - IC50
  - Difusão × fluxo

## Software <!-- markmap: fold -->

### Detecção de gotas   

- Estratégias:
  - Detecção por threshold
  - Detecção por bordas + Hough
  - Separação por watershed
  - Tracking frame a frame
- Parâmetros:
  - Tamanho mínimo/máximo
  - Circularidade
  - Velocidade da gota
- Ferramentas:
  - OpenCV
  - scikit-image
  - TrackPy

### Métricas Extraídas

- Número de gotas
- Frequência de geração (Hz)
- Tamanho médio
- Distribuição de tamanhos
- Velocidade / taxa de fluxo
- Coalescência ou ruptura

### Contagem de Células

#### Segmentação Celular

- Métodos:
  - Threshold + morfologia
  - Contornos + watershed
  - Deep learning (U-Net)
- Correções:
  - Separação de células grudadas
  - Eliminação de artefatos
- Modos:
  - Campo claro
  - Fluorescência nuclear (DAPI)

### Métricas

- Número total de células
- Área média
- Circularidade
- Densidade por região
- Crescimento temporal

### Análise de Esferoides

#### Segmentação de Esferoides   

- Contorno externo automático
- Watershed para múltiplos clusters
- Detecção de núcleo vs periferia
- Separação de fundo iluminado irregular

#### Quantificação

- Área do esferoide (μm²)
- Diâmetro equivalente
- Volume estimado (modelo esférico)
- Textura (Haralick)
- Circularidade

#### Crescimento Temporal

- Tracking do esferoide ao longo do tempo
- Crescimento relativo (%)
- Curvas de crescimento (logístico / Gompertz)
- Taxa de expansão radial

### Processamento de Vídeos

- Extração de frames
- Tracking:
  - Gotas
  - Células
  - Esferoides
- Medidas ao longo do tempo
- Detecção de eventos:
  - Coalescência (gotas)
  - Apoptose (células)
  - Quebra / deformação (esferoides)

### Visualização e Relatórios

- Overlays:
  - Contornos detectados
  - Centros de massa
  - Numeração das gotas
  - Escalas em μm
- Geração automática de:
  - GIFs
  - Vídeos anotados
  - Gráficos (tamanho × tempo)
  - Tabelas de métricas
