# Exercício Teórico - Aula 08 (Slide 23)

**Pergunta 1:** Quais seriam os valores ideais de Eye, Center e Up para o `gluLookAt` observar um objeto na origem de cima?
**Resposta:** - **Eye (Posição):** (0, 10, 0) - Posiciona a câmera no alto do eixo Y.
- **Center (Alvo):** (0, 0, 0) - Aponta para a origem.
- **Up (Orientação):** (0, 0, -1) - Define o topo da câmera apontando para o eixo Z negativo (evitando o alinhamento com o eixo Y).

**Pergunta 2:** Para o Braço Robótico (Base e Braço), qual a ordem correta ($T \rightarrow R$ ou $R \rightarrow T$) no código para que o braço rotacione em torno de sua própria junta após ser movido para a ponta da base?
**Resposta:** A ordem correta é **Translação seguida de Rotação ($T \rightarrow R$)**. 
**Justificativa:** No OpenGL Moderno (e GLM), as transformações são aplicadas da última para a primeira (ordem inversa à escrita). Para que o braço gire corretamente, ele deve ser rotacionado em seu sistema local (origem) e, em seguida, transladado para a posição final na extremidade da base.