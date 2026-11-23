# GUD FNF — Nintendo DS / DS Lite

Homebrew de ritmo para Nintendo DS original e DS Lite, compilado com devkitPro/devkitARM e libnds.

## Conteúdo desta build

- Menu com Story Mode, Freeplay e Créditos.
- Gameplay de quatro pistas com notas em movimento.
- Julgamento de acertos, pontuação, combo e misses.
- Controles pelo direcional, A/B/X/Y e tela de toque.
- Efeito sonoro rítmico em loop para validar a reprodução de áudio no hardware.

## Limitações desta primeira build jogável

Esta é a primeira versão de gameplay no DS, ainda não é uma conversão completa do mod Mid-Fight Masses. A faixa de áudio é um loop de teste sintetizado e o chart atual é um padrão de treino gerado no código. Os sprites, cenários e as músicas originais (Parish, Worship, Zavodila e Gospel) ainda precisam ser convertidos e integrados para o hardware do DS. Sustains, animações e seleção individual de músicas também ainda não estão implementados.

## Compilar

O GitHub Actions instala `nds-dev`, converte o áudio de teste para o NitroFS, executa `make` e publica o arquivo `GUD_FNF_DS.nds` como artefato.

Criador desta versão: ivtguioliveira8.
