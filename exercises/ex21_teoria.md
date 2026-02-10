# Exercício Teórico - Aula 21 (Slide 16)

**Pergunta:** O que acontece quando você ativa o Back-Face Culling e define que a face frontal (Front) é a que segue a ordem horária (`GL_CW`) em um objeto modelado em sentido anti-horário (`CCW`)?

**Resposta:** O OpenGL irá descartar as faces que estão voltadas para o observador e renderizar apenas as faces internas (traseiras). Visualmente, o objeto parecerá "oco" ou "aberto", pois as superfícies que deveriam ser visíveis serão removidas pelo teste de Culling, permitindo que o usuário veja através do objeto para a parede interna oposta. Isso demonstra que o Culling depende inteiramente da consistência entre a modelagem dos vértices e a configuração do `glFrontFace`.