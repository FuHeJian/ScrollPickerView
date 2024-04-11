@Suppress("DSL_SCOPE_VIOLATION") // TODO: Remove once KTIJ-19369 is fixed
plugins {
    alias(libs.plugins.androidApplication)
    alias(libs.plugins.kotlinAndroid)
}


android {

    namespace = "com.fhj.mvi_demo"
    compileSdk = 34

    defaultConfig {
        applicationId = "com.fhj.mvi_demo"
        minSdk = 26
        targetSdk = 34
        versionCode = 1
        versionName = "1.0"

        testInstrumentationRunner = "androidx.test.runner.AndroidJUnitRunner"

        ndkVersion = "26.0.10792818"

        ndk {
            abiFilters.clear()
            abiFilters.addAll(arrayListOf("arm64-v8a"))
        }

        externalNativeBuild {
            cmake {
                this.cppFlags("-fdeclspec")
            }
        }

    }

    externalNativeBuild {
        cmake {
            this.path("src/main/cpp/CMakeLists.txt")
        }
    }

    buildTypes {
        release {
            isMinifyEnabled = false
            proguardFiles(
                getDefaultProguardFile("proguard-android-optimize.txt"),
                "proguard-rules.pro"
            )
        }
    }


    compileOptions {
        sourceCompatibility = JavaVersion.VERSION_1_8
        targetCompatibility = JavaVersion.VERSION_1_8
    }
    kotlinOptions {
        jvmTarget = "1.8"
    }
    buildFeatures {
        dataBinding = true
        viewBinding = true
    }
    dataBinding {
        this.enable = true
        this.addDefaultAdapters = true
    }
}

dependencies {

    implementation(libs.core.ktx)
    implementation(libs.appcompat)
    implementation(libs.material)
    implementation(libs.mavericks)
    implementation(libs.cameraxView)
    implementation(libs.cameraxLifeCycle)
    implementation(libs.cameraxcamera2)
    implementation(libs.cameraxcore)
    implementation(libs.androidXWebkit)
    implementation(libs.glide)
    implementation(project(":extview"))
    implementation("androidx.constraintlayout:constraintlayout:2.2.0-alpha11")
    testImplementation(libs.junit)
    androidTestImplementation(libs.androidx.test.ext.junit)
    androidTestImplementation(libs.espresso.core)

}