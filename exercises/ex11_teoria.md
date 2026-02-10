# Exercício Teórico - Aula 11 (Slide 34)

**Pergunta:** Por que um objeto se comporta de forma "estranha" no modo "GL_SMOOTH" (Gouraud) em comparação com o modo "GL_FLAT"?

**Resposta:** No modo `GL_FLAT`, o OpenGL calcula a iluminação uma única vez por face, mantendo as arestas nítidas. No modo `GL_SMOOTH`, a intensidade da luz é calculada nos vértices e interpolada linearmente através da face. O comportamento "estranho" ocorre principalmente em quinas vivas (como um cubo) onde os vértices compartilham a mesma normal média das faces adjacentes. O hardware tenta suavizar uma transição que deveria ser brusca, resultando em manchas de luz irreais e perda da definição geométrica das bordas (o chamado efeito de "sabão").