# GUD FNF — Nintendo DS / DS Lite

Projeto homebrew para o Nintendo DS original e DS Lite. O alvo de compilação é um arquivo `.nds`, usando devkitPro/devkitARM e libnds.

## Estado atual

Esta primeira base é um **protótipo de ritmo compilável**, não uma conversão completa do FNF:
- menu inicial;
- quatro pistas com entrada pelas setas e pela tela touch;
- notas de jogador e notas automáticas do oponente;
- pontuação, combo e misses;
- pausa pelo botão START;
- tela de resultado.

A música e os gráficos originais ainda não foram integrados. O Nintendo DS não reproduz OGG diretamente; os áudios precisam ser convertidos e integrados por uma biblioteca de áudio compatível. Imagens PNG/XML também precisam ser convertidas para formatos adequados ao hardware.

## Compilar

O GitHub Actions instala o pacote `nds-dev` e executa `make`. O arquivo gerado é `GUD_FNF_DS.nds`, publicado como artefato da execução.

Também é possível compilar localmente com devkitPro instalado:

```sh
cd Nintendo-DS
make
```

## Teste

Abra o artefato `.nds` no melonDS. A compilação bem-sucedida confirma que o arquivo foi produzido; a execução no emulador ou em um DS real precisa ser testada separadamente.

Criador: ivtguioliveira8.
