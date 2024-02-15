
package com.selex.bssStation.core.utils.network

import android.content.Context
import com.selex.bssStation.core.utils.SharedPreferences
import okhttp3.Interceptor
import okhttp3.Response
import javax.inject.Inject
import javax.inject.Singleton

@Singleton
class NetworkInterceptor @Inject constructor(private var sharedPreferences: SharedPreferences, var context: Context): Interceptor {


    override fun intercept(chain: Interceptor.Chain): Response {
        val request = chain.request()
        val urlEndCode = request.url.toString().replace("%2C", ",")
        val newRequest = request.newBuilder()
        if (sharedPreferences.getToken().isNotEmpty()) {
            newRequest.header("Authorization", "Bearer" + " " + sharedPreferences.getToken())
        }
        newRequest.url(urlEndCode)


        return chain.proceed(newRequest.build())

    }

}