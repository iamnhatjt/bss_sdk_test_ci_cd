package com.selex.bssStation.screen.login.loginQr

import android.app.Activity
import android.content.Intent
import android.graphics.Bitmap
import android.os.Bundle
import android.os.CountDownTimer
import android.view.View
import androidx.activity.viewModels
import androidx.databinding.DataBindingUtil
import com.bumptech.glide.Glide
import com.bumptech.glide.load.engine.DiskCacheStrategy
import com.bumptech.glide.request.RequestOptions.bitmapTransform
import com.google.zxing.BarcodeFormat
import com.google.zxing.EncodeHintType
import com.google.zxing.MultiFormatWriter
import com.journeyapps.barcodescanner.BarcodeEncoder
import com.selex.bssStation.R
import com.selex.bssStation.core.utils.Constants
import com.selex.bssStation.core.utils.TAG
import com.selex.bssStation.core.utils.NetworkResult
import com.selex.bssStation.databinding.ActivityLoginQractivityBinding
import com.selex.bssStation.screen.Screen
import com.selex.bssStation.screen.ScreenManager
import com.selex.bssStation.screen.base.BaseActivity
import com.selex.bssStation.screen.home.MainActivity
import com.selex.bssStation.screen.login.loginAccount.LoginAccountActivity
import com.selex.bssStation.service.NoActionCommon
import com.selex.bssStation.service.getSerialNumberBssFromJNI
import dagger.hilt.android.AndroidEntryPoint
import jp.wasabeef.glide.transformations.BlurTransformation
import java.util.UUID

@AndroidEntryPoint
class LoginQRActivity : BaseActivity() {
    private lateinit var _binding: ActivityLoginQractivityBinding
    private val viewModel: LoginQRViewModel by viewModels()
    private var idGenerated = 0

    companion object {
        fun startActivity(activity: Activity, bundle: Bundle? = null) {
            val intent = Intent(activity, LoginQRActivity::class.java)
            if (bundle != null) {
                intent.putExtras(bundle)
            }
            activity.startActivity(intent)
        }
    }

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)

        _binding = DataBindingUtil.setContentView(this, R.layout.activity_login_qractivity)
        NoActionCommon.instance.startNoAction()
        configView()
        bindData()
        // abc()
        viewModel.generateQr()
    }

    override fun onResume() {
        super.onResume()
        ScreenManager.currentScreen = Screen.LOGIN_QR
    }

    private fun configView() {
        overridePendingTransition(R.anim.right_in, R.anim.left_out)

        _binding.btnBack.setOnClickListener {
            onBackPressedDispatcher.onBackPressed()
        }
        _binding.layoutButtonLoginAccount.setOnClickListener {
            LoginAccountActivity.startActivity(this)
        }
        _binding.btnError.setOnClickListener {
            viewModel.generateQr()
        }
    }

    override fun finish() {
        super.finish()
        NoActionCommon.instance.cancelTimer()
    }

    private fun abc() {
        _binding.imgQr.setImageBitmap(getSerialNumberBssFromJNI()?.let { generateQrImage(it) })
    }

    private fun bindData() {
        viewModel.qrResponse.observe(this) { result ->
            when (result) {
                is NetworkResult.Loading -> {
                    _binding.apply {
                        blurQrWithRandomSerial()
                        layoutError.visibility = View.GONE
                        layoutLoading.visibility = View.VISIBLE
                    }
                }

                is NetworkResult.Success -> {
                    if (result.data?.code?.isNotBlank() == true && result.data.id != null) {
                        _binding.apply {
                            layoutLoading.visibility = View.GONE
                            layoutError.visibility = View.GONE
                            imgQr.setImageBitmap(generateQrImage(result.data.code))

                            idGenerated = result.data.id
                            viewModel.pollingQr(result.data.id)
                        }
                    } else {
                        _binding.apply {
                            blurQrWithRandomSerial()
                            layoutLoading.visibility = View.GONE
                            tvError.text = getString(R.string.can_not_generate_qr_code)
                            tvInButtonError.text = getString(R.string.regenerate_qr_code)
                            layoutError.visibility = View.VISIBLE
                        }
                    }
                }

                is NetworkResult.Error -> {
                    _binding.apply {
                        blurQrWithRandomSerial()
                        layoutLoading.visibility = View.GONE
                        tvError.text = getString(R.string.can_not_generate_qr_code)
                        tvInButtonError.text = getString(R.string.regenerate_qr_code)
                        layoutError.visibility = View.VISIBLE
                    }
                }
            }
        }

        viewModel.pollingQrResponse.observe(this) {
            when (it.data?.status) {
                Constants.STATUS_NEW -> {
                    timerPollingResultQr(2000)
                }
                Constants.STATUS_SCAN -> {
                    _binding.apply {
                        layoutScanQrCode.visibility = View.GONE
                        tvName.text = it.data.user?.fullName
                        if (it.data.user?.imageUrl.toString().isNotBlank()) {
                            Glide.with(applicationContext).load(it.data.user?.imageUrl.toString()).diskCacheStrategy(
                                DiskCacheStrategy.ALL).placeholder(R.drawable.avtar_defaul).error(R.drawable.img_banner).into(imgAvatar)
                        }
                        layoutScanQrCodeSuccess.visibility = View.VISIBLE
                    }
                    timerPollingResultQr(2000)
                }
                Constants.STATUS_CONFIRM -> {
                    viewModel.getProfile()
                }
                Constants.STATUS_EXPIRED -> {
                    _binding.apply {
                        layoutScanQrCodeSuccess.visibility = View.GONE
                        layoutScanQrCode.visibility = View.VISIBLE

                        blurQrWithRandomSerial()
                        tvError.text = getString(R.string.qr_expired)
                        tvInButtonError.text = getString(R.string.get_new_code)
                        layoutError.visibility = View.VISIBLE
                    }
                }
                Constants.STATUS_REJECT -> {
                    _binding.apply {
                        layoutScanQrCodeSuccess.visibility = View.GONE
                        layoutScanQrCode.visibility = View.VISIBLE

                        blurQrWithRandomSerial()
                        tvError.text = getString(R.string.qr_expired)
                        tvInButtonError.text = getString(R.string.get_new_code)
                        layoutError.visibility = View.VISIBLE
                    }
                }
            }
        }

        viewModel.responseUser.observe(this) {
            if (it.data != null) {
                MainActivity.startActivity(this, model = it.data)
            }
        }
    }

    private fun blurQrWithRandomSerial() {
        val bitmap = generateQrImage(UUID.randomUUID().toString())
        Glide.with(applicationContext)
            .load(bitmap)
            .apply(bitmapTransform(BlurTransformation(10, 1)))
            .into(_binding.imgQr)
    }

    private fun generateQrImage(serialStation: String): Bitmap? {
        val multiFormatWriter = MultiFormatWriter();
        return try {
            val hintMap = mapOf(EncodeHintType.MARGIN to 2)
            val bitMatrix = multiFormatWriter.encode(serialStation, BarcodeFormat.QR_CODE, 400, 400, hintMap)
            val barcodeEncoder = BarcodeEncoder();
            barcodeEncoder.createBitmap(bitMatrix);
        } catch (e: Exception) {
            null
        }
    }

    private fun timerPollingResultQr(time: Long) {
        object : CountDownTimer(time, 1000) {
            override fun onTick(millisUntilFinished: Long) {}
            override fun onFinish() {
                viewModel.pollingQr(idGenerated)
            }
        }.start()
    }

}