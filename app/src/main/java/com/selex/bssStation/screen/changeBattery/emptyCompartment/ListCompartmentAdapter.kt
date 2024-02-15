package com.selex.bssStation.screen.changeBattery.emptyCompartment

import android.annotation.SuppressLint
import android.os.CountDownTimer
import android.view.LayoutInflater
import android.view.View
import android.view.ViewGroup
import android.widget.TextView
import androidx.constraintlayout.widget.ConstraintLayout
import androidx.recyclerview.widget.RecyclerView
import com.selex.bssStation.R


class ListCompartmentAdapter(private val animatedIndex: Int): RecyclerView.Adapter<ListCompartmentAdapter.ChangeBatteryViewHolder>() {

    private val listBatteryCompartment = (0..19).toList()
    private var timerAnimation: CountDownTimer? = null
    override fun onCreateViewHolder(parent: ViewGroup, viewType: Int): ChangeBatteryViewHolder {
        val view: View = LayoutInflater.from(parent.context).inflate(R.layout.item_battery_compartment, parent, false)

        // Change height item fit 1/5 of parent (5 rows, 4 columns)
        val layoutParams = view.layoutParams
        layoutParams.height = parent.measuredHeight / 5
        view.layoutParams = layoutParams

        return ChangeBatteryViewHolder(view)
    }

    override fun getItemCount(): Int {
        return listBatteryCompartment.size
    }

    override fun onBindViewHolder(holder: ChangeBatteryViewHolder, position: Int) {
        holder.bind(listBatteryCompartment[position])

        if (position == 0) {
            holder.layoutBatteryCompartment.setBackgroundResource(R.drawable.img_item_bettery_compartment)
            holder.tvCircle.visibility = View.GONE
            holder.tvNumber.visibility = View.GONE
            holder.tvLed.visibility = View.INVISIBLE
        } else {
            holder.layoutBatteryCompartment.setBackgroundResource(R.drawable.bg_battery_compartment_black)
            holder.tvCircle.setBackgroundResource(R.drawable.bg_circle_number)
            holder.tvNumber.setTextColor(holder.itemView.resources.getColor(R.color.color_4A4D55))
            holder.tvLed.setBackgroundResource(R.drawable.bg_black_led)
        }

        if (position == animatedIndex) {
            holder.tvNumber.textSize = 42F
            timerAnimation = object : CountDownTimer(1400, 700) {
                override fun onTick(p0: Long) {
                    if(p0 > 700) {
                        holder.layoutBatteryCompartment.setBackgroundResource(R.drawable.bg_battery_compartment_blue)
                        holder.tvCircle.setBackgroundResource(R.color.color_B4C5FF)
                        holder.tvNumber.setTextColor(holder.itemView.resources.getColor(R.color.color_2656C8, null))
                        holder.tvLed.setBackgroundResource(R.drawable.bg_red_led)
                    } else {
                        holder.layoutBatteryCompartment.setBackgroundResource(R.drawable.bg_battery_compartment_black)
                        holder.tvCircle.setBackgroundResource(R.drawable.bg_circle_number)
                        holder.tvNumber.setTextColor(holder.itemView.resources.getColor(R.color.color_4A4D55, null))
                        holder.tvLed.setBackgroundResource(R.drawable.bg_black_led)
                    }
                }

                override fun onFinish() {
                    this.start()
                }
            }.start()
        }
    }

    fun cancelTimerAnimation() {
        timerAnimation?.cancel()
    }

    class ChangeBatteryViewHolder(itemView: View): RecyclerView.ViewHolder(itemView) {
        val layoutBatteryCompartment: ConstraintLayout = itemView.findViewById(R.id.layout_battery_compartment)
        val tvNumber: TextView = itemView.findViewById(R.id.tv_number)
        val tvLed: TextView = itemView.findViewById(R.id.tv_led)
        val tvCircle: TextView = itemView.findViewById(R.id.tv_circle)

        @SuppressLint("SetTextI18n")
        fun bind(index: Int) {
            if (index.toString().length == 1) {
                tvNumber.text = "0${index}"
            } else {
                tvNumber.text = index.toString()
            }
        }
    }

}