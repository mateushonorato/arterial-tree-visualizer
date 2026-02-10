# Exercício Teórico - Aula 13 (Slide 18)

**Pergunta:** Discuta por que o destaque especular (o brilho) no Gouraud Shading (GL_SMOOTH) parece "correr" ou desaparecer de forma não natural, ilustrando a necessidade do Shading de Phong.

**Resposta:**
O problema reside no momento em que o cálculo de iluminação é realizado. No **Gouraud Shading**, a iluminação (incluindo a componente especular) é calculada apenas nos **vértices** e depois interpolada linearmente pelas faces. 

Como o destaque especular é um fenómeno muito localizado (depende de um expoente de brilho alto), se o ponto de brilho máximo cair no centro de um polígono e não próximo dos seus vértices, os valores calculados nos vértices serão baixos. Durante a interpolação, o brilho será "diluído" ou desaparecerá completamente. Quando o objeto se move, o brilho parece "saltar" de um vértice para outro de forma abrupta. 

O **Phong Shading** resolve este problema ao interpolar as **normais** através da face e realizar o cálculo de iluminação para cada fragmento (pixel), garantindo que o brilho especular seja representado corretamente em qualquer ponto da superfície, independentemente da densidade da malha.