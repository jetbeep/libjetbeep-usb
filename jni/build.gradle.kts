subprojects {
    version = file("../../VERSION").readText()
}

val cmake = tasks.register<Exec>("cmake") {
    commandLine = listOf("cmake", "--build", ".", "--target", "jetbeep-jni", "--")
    workingDir = file("../build")
}

tasks.register("build") {
    dependsOn(cmake)
}
