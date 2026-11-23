# GUD FNF PSP

Projeto de port homebrew de um jogo de ritmo inspirado em Friday Night Funkin para PSP/PPSSPP.

**Criador do projeto e responsável pela integração:** ivtguioliveira8  
**Versão do projeto:** 0.2-dev

## Estado real

Este repositório ainda NÃO contém um port completo. O código PSP atual é um protótipo de gameplay em texto, com quatro lanes, pontuação, combo, misses e controles do D-pad. A compilação gera um EBOOT.PBP de teste; isso não equivale ao jogo completo nem comprova compatibilidade de todas as funcionalidades.

O pacote de recursos enviado pelo usuário foi inspecionado localmente: ZIP válido, cerca de 40 MB e 143 entradas. Ele inclui imagens, spritesheets, fontes, diálogos, músicas OGG e um chart JSON de Tutorial Remix. Os arquivos binários desse pacote ainda precisam ser adicionados ao repositório; eles não foram enviados por esta atualização do GitHub.

## Compilar

O workflow GitHub Actions usa a imagem `pspdev/pspdev:latest` e executa `make`. O artefato compilado, quando o workflow termina com sucesso, fica disponível na aba Actions.

## Próximas etapas técnicas

1. Importar os recursos originais para `assets/`, preservando os nomes e pastas.
2. Converter/decodificar texturas para um formato suportado pela rotina gráfica PSP.
3. Preparar áudio compatível com PSP e implementar reprodução sincronizada.
4. Implementar leitor dos charts e validar tempos/BPM.
5. Implementar menu, seleção de músicas, personagens, cenários, HUD e notas animadas.
6. Testar o EBOOT no PPSSPP e em hardware PSP, registrando limitações.

## Créditos

Consulte [CREDITS.md](CREDITS.md). Não foram inventados créditos de artistas/charters que não constavam no pacote recebido.
