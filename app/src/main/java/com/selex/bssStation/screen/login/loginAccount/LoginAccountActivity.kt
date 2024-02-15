package com.selex.bssStation.screen.login.loginAccount

import android.app.Activity
import android.content.Intent
import android.os.Bundle
import android.text.Editable
import android.text.TextWatcher
import android.view.View
import android.view.inputmethod.InputMethodManager
import androidx.activity.viewModels
import androidx.databinding.DataBindingUtil
import com.selex.bssStation.R
import com.selex.bssStation.core.utility.BottomDialogFragment
import com.selex.bssStation.core.utils.Constants
import com.selex.bssStation.core.utils.TAG
import com.selex.bssStation.core.utils.NetworkResult
import com.selex.bssStation.databinding.ActivityLoginAccountBinding
import com.selex.bssStation.model.user.defaultGuarantorModel
import com.selex.bssStation.screen.Screen
import com.selex.bssStation.screen.ScreenManager
import com.selex.bssStation.screen.base.BaseActivity
import com.selex.bssStation.screen.home.MainActivity
import com.selex.bssStation.service.NoActionCommon
import dagger.hilt.android.AndroidEntryPoint

@AndroidEntryPoint
class LoginAccountActivity : BaseActivity() {
    private lateinit var _binding: ActivityLoginAccountBinding
    private lateinit var bottomDialogFragment: BottomDialogFragment
    private val viewModel: LoginAccountViewModel by viewModels()

    companion object {
        fun startActivity(activity: Activity, bundle: Bundle? = null) {
            val intent = Intent(activity, LoginAccountActivity::class.java)
            if (bundle!= null) {
                intent.putExtras(bundle)
            }
            activity.startActivity(intent)
        }
    }

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        _binding = DataBindingUtil.setContentView(this, R.layout.activity_login_account)
        bottomDialogFragment = _binding.bottomDialogFragment.getFragment()
        NoActionCommon.instance.startNoAction()
        configView()
    }

    override fun onResume() {
        super.onResume()
        ScreenManager.currentScreen = Screen.LOGIN_ACCOUNT
    }

    private fun configView() {
        overridePendingTransition(R.anim.right_in, R.anim.left_out)

        _binding.btnBack.setOnClickListener {
            onBackPressedDispatcher.onBackPressed()
        }
        _binding.btnLogin.setOnClickListener {
            if (!_binding.edtUsername.text.isNullOrEmpty() && !_binding.edtPassword.text.isNullOrEmpty()) {
                hideKeyBoard()

                val username = _binding.edtUsername.text.toString()
                val password = _binding.edtPassword.text.toString()

                if(username == Constants.GUARANTOR_USERNAME_LOCAL && password == Constants.GUARANTOR_PASSWORD_LOCAL) {
                    MainActivity.startActivity(
                        activity = this@LoginAccountActivity,
                        bundle = null,
                        model = defaultGuarantorModel
                    )
                } else if (_binding.progressBarLoginAccount.visibility != View.VISIBLE) {
                    viewModel.login(username, password)
                }
            }
        }
        observeData()
        focusEdittext()
        _binding.imgCloseError.setOnClickListener {
            _binding.layoutErrorAccountOrPassword.visibility = View.GONE
            _binding.edtUsername.setText("")
            _binding.edtPassword.setText("")
            focusEdittext()
        }
        _binding.layoutLoginAccount.setOnClickListener {
            hideKeyBoard()
        }
        handleDisableButton()
    }

    private fun hideKeyBoard() {
        val view = this.currentFocus
        if (view != null) {
            val imm = getSystemService(INPUT_METHOD_SERVICE) as InputMethodManager
            imm.hideSoftInputFromWindow(view.windowToken, 0)
        }
    }

    private fun focusEdittext() {
        _binding.edtUsername.requestFocus()
        val imm = getSystemService(INPUT_METHOD_SERVICE) as InputMethodManager
        imm.showSoftInput(_binding.edtUsername, InputMethodManager.SHOW_IMPLICIT)
    }

    override fun finish() {
        super.finish()
        overridePendingTransition(R.anim.left_in, R.anim.right_out)
    }


    private fun observeData() {
        viewModel.response.observe(this) { result ->
            when(result) {
                is NetworkResult.Loading -> {
                    _binding.apply {
                        tvLogin.visibility = View.GONE
                        imgIconBtnLogin.visibility = View.GONE
                        progressBarLoginAccount.visibility = View.VISIBLE
                    }
                }
                else -> {
                    _binding.apply {
                        tvLogin.visibility = View.VISIBLE
                        imgIconBtnLogin.visibility = View.VISIBLE
                        progressBarLoginAccount.visibility = View.GONE
                    }
                    if (!result.data?.id_token.isNullOrEmpty()) {
                        viewModel.getProfile()
                    } else if (result.error?.status == 401) {
                        bottomDialogFragment.show(title =  getString(R.string.login_not_success), message = getString(R.string.error_info_account_or_password))
                    } else {
                        bottomDialogFragment.show(title =  getString(R.string.login_not_success), message = getString(R.string.login_not_success))
                    }
                }
            }
        }

        viewModel.responseUser.observe(this) {
            if (it.data != null) {
                MainActivity.startActivity(this, model = it.data)
            } else {
                bottomDialogFragment.show(title =  getString(R.string.login_not_success), message = getString(R.string.login_not_success))
            }
        }
    }

    private fun handleDisableButton() {
        _binding.edtUsername.addTextChangedListener(object : TextWatcher {
            override fun beforeTextChanged(p0: CharSequence?, p1: Int, p2: Int, p3: Int) {}

            override fun onTextChanged(p0: CharSequence?, p1: Int, p2: Int, p3: Int) {

                if (_binding.edtUsername.text.isNullOrEmpty() || _binding.edtPassword.text.isNullOrEmpty()) {
                    _binding.layoutBtnLogin.setBackgroundResource(R.drawable.bg_button_disable)
                    _binding.imgIconBtnLogin.setImageResource(R.drawable.img_icon_left_login_gray)
                    _binding.tvLogin.setTextColor(resources.getColor(R.color.color_FFC7C6CA))
                } else {
                    _binding.layoutBtnLogin.setBackgroundResource(R.drawable.bg_button_blue)
                    _binding.imgIconBtnLogin.setImageResource(R.drawable.img_icon_left_login)
                    _binding.tvLogin.setTextColor(resources.getColor(R.color.white))
                }
            }
            override fun afterTextChanged(p0: Editable?) {
                NoActionCommon.instance.startNoAction()
            }
        })

        _binding.edtPassword.addTextChangedListener(object : TextWatcher {
            override fun beforeTextChanged(p0: CharSequence?, p1: Int, p2: Int, p3: Int) {}

            override fun onTextChanged(p0: CharSequence?, p1: Int, p2: Int, p3: Int) {
                if (_binding.edtUsername.text.isNullOrEmpty() || _binding.edtPassword.text.isNullOrEmpty()) {
                    _binding.layoutBtnLogin.setBackgroundResource(R.drawable.bg_button_disable)
                    _binding.imgIconBtnLogin.setImageResource(R.drawable.img_icon_left_login_gray)
                    _binding.tvLogin.setTextColor(resources.getColor(R.color.color_FFC7C6CA))
                } else {
                    _binding.layoutBtnLogin.setBackgroundResource(R.drawable.bg_button_blue)
                    _binding.imgIconBtnLogin.setImageResource(R.drawable.img_icon_left_login)
                    _binding.tvLogin.setTextColor(resources.getColor(R.color.white))
                }
            }
            override fun afterTextChanged(p0: Editable?) {
                NoActionCommon.instance.startNoAction()
            }
        })
    }
}