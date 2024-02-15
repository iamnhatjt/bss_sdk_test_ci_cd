package com.selex.bssStation.core.utility

import android.os.Bundle
import android.os.CountDownTimer
import android.view.LayoutInflater
import android.view.View
import android.view.ViewGroup
import android.view.animation.AccelerateInterpolator
import android.view.animation.Animation
import android.view.animation.Animation.AnimationListener
import android.view.animation.TranslateAnimation
import androidx.annotation.DrawableRes
import androidx.fragment.app.Fragment
import com.selex.bssStation.R
import com.selex.bssStation.databinding.FragmentBottomDialogBinding

class BottomDialogFragment : Fragment(R.layout.fragment_bottom_dialog) {

    private lateinit var _binding: FragmentBottomDialogBinding
    private var timerCount = 4000L
    private var countDownTimer: CountDownTimer? = null

    private fun showFragment() {
        countDownTimer?.cancel()
        countDownTimer = object : CountDownTimer(timerCount, timerCount) {
            override fun onTick(millisUntilFinished: Long) {

            }
            override fun onFinish() {
                hideFragment()
            }
        }.start()
    }

    override fun onCreateView(
        inflater: LayoutInflater,
        container: ViewGroup?,
        savedInstanceState: Bundle?,
    ): View {
        _binding = FragmentBottomDialogBinding.inflate(inflater, container, false)
        _binding.iconClose.setOnClickListener {
            hideFragment()
        }
        return _binding.root
    }

    fun show(
        @DrawableRes iconStatus: Int = R.drawable.img_icon_error,
        title: String = getString(R.string.station_maintenance),
        message: String = getString(R.string.come_back_later),
        timerShow: Long = 6000L
    ) {
        _binding.apply {
            icStatus.setImageResource(iconStatus)
            tvTitle.text = title
            tvMessage.text = message
        }
        showFragment(
            onAnimationEnd = {
                timerCount = timerShow
                showFragment()
            }
        )
    }

    private fun showFragment(onAnimationEnd: () -> Unit) {
        _binding.bottomDialogFragment.clearAnimation()
        val transitionEnter = TranslateAnimation(
            0f,
            0f,
            _binding.bottomDialogFragment.height.toFloat(),
            0f
        ).apply {
            duration = 300
            interpolator = AccelerateInterpolator(0.5f)
            setAnimationListener(
                object : AnimationListener {
                    override fun onAnimationStart(animation: Animation?) {
                    }

                    override fun onAnimationEnd(animation: Animation?) {
                        _binding.bottomDialogFragment.visibility = View.VISIBLE
                        onAnimationEnd()
                    }

                    override fun onAnimationRepeat(animation: Animation?) {
                    }
                }
            )
        }
        _binding.bottomDialogFragment.startAnimation(transitionEnter)
    }

    private fun hideFragment() {
        _binding.bottomDialogFragment.clearAnimation()
        val transitionExit = TranslateAnimation(
            0f,
            0f,
            0f,
            _binding.bottomDialogFragment.height.toFloat(),
        ).apply {
            duration = 300
            interpolator = AccelerateInterpolator(0.5f)
            setAnimationListener(
                object : AnimationListener {
                    override fun onAnimationStart(animation: Animation?) {
                        countDownTimer?.cancel()
                    }

                    override fun onAnimationEnd(animation: Animation?) {
                        _binding.bottomDialogFragment.visibility = View.INVISIBLE
                    }

                    override fun onAnimationRepeat(animation: Animation?) {
                    }
                }
            )
        }
        _binding.bottomDialogFragment.startAnimation(transitionExit)
    }
}