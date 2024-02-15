package com.selex.bssStation.di


import com.selex.bssStation.core.utils.SharedPreferences
import android.app.Application
import android.content.Context
import com.chuckerteam.chucker.api.ChuckerInterceptor
import com.selex.bssStation.App.Companion.context
import com.selex.bssStation.core.service.AccountService
import com.selex.bssStation.core.service.BssService
import com.selex.bssStation.core.service.ChangeBatteryService
import com.selex.bssStation.core.service.UploadFileService
import com.selex.bssStation.core.utils.*
import com.selex.bssStation.core.utils.network.NetworkInterceptor
import dagger.Module
import dagger.Provides
import dagger.hilt.InstallIn
import dagger.hilt.android.qualifiers.ApplicationContext
import dagger.hilt.components.SingletonComponent
import okhttp3.OkHttpClient
import okhttp3.logging.HttpLoggingInterceptor
import retrofit2.Retrofit
import retrofit2.converter.gson.GsonConverterFactory
import java.util.concurrent.TimeUnit
import javax.inject.Named
import javax.inject.Singleton

@Module
@InstallIn(SingletonComponent::class)
object NetworkModule {

    @Provides
    @Singleton
    fun provideContext(application: Application): Context {
        return application
    }

    @Singleton
    @Provides
    fun  provideSharedPreferences(@ApplicationContext context: Context): SharedPreferences {
        return SharedPreferences(context)
    }

    @Provides
    fun provideHTTPLoggingInterceptor(): HttpLoggingInterceptor {
        val interceptor = HttpLoggingInterceptor()
        interceptor.setLevel(HttpLoggingInterceptor.Level.BODY)
        return interceptor
    }

    @Provides
    fun provideOkHttpClient(networkInterceptor: NetworkInterceptor,
                            loggingInterceptor: HttpLoggingInterceptor,
                            ): OkHttpClient {
        return OkHttpClient.Builder()
            .addInterceptor(loggingInterceptor)
            .addInterceptor(networkInterceptor)
            .addInterceptor(ChuckerInterceptor(context!!))
            .readTimeout(30, TimeUnit.SECONDS)
            .connectTimeout(30, TimeUnit.SECONDS)
            .build()
    }

    @Provides
    @Named(ModuleConstant.SELEX_API_RETROFIT)
    fun provideSelexRetrofit(okHttpClient: OkHttpClient): Retrofit {
        return Retrofit.Builder()
            .baseUrl(Constants.BASE_URL)
            .addConverterFactory(GsonConverterFactory.create())
            .client(okHttpClient)
            .build()
    }

    @Provides
    fun provideBssService(@Named(ModuleConstant.SELEX_API_RETROFIT) retrofit: Retrofit): BssService {
        return retrofit.create(BssService::class.java);
    }

    @Provides
    fun provideAccountService(@Named(ModuleConstant.SELEX_API_RETROFIT) retrofit: Retrofit): AccountService {
        return retrofit.create(AccountService::class.java);
    }
    @Provides
    fun provideChangeBatteryService(@Named(ModuleConstant.SELEX_API_RETROFIT) retrofit: Retrofit): ChangeBatteryService {
        return retrofit.create(ChangeBatteryService::class.java);
    }

    @Provides
    @Named(ModuleConstant.FILE_API_RETROFIT)
    fun provideFileUploadRetrofit(): Retrofit {
        return Retrofit.Builder()
            .baseUrl(Constants.UPLOAD_FILE_BASE_URL)
            .addConverterFactory(GsonConverterFactory.create())
            .build()
    }

    @Provides
    fun provideFileUploadService(@Named(ModuleConstant.FILE_API_RETROFIT) retrofit: Retrofit): UploadFileService {
        return retrofit.create(UploadFileService::class.java);
    }

}