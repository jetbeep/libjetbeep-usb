plugins {
    `java-library`
}

java {
    withJavadocJar()
    withSourcesJar()
}

tasks {
    jar {
        manifest {
            attributes(
                mapOf("Implementation-Title" to project.name,
                      "Implementation-Version" to project.version)
            )
        }
    }
}