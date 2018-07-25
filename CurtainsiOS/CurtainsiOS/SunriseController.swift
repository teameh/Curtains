//
//  SunriseController.swift
//  CurtainsiOS
//
//  Created by Tieme van Veen on 15/07/2018.
//  Copyright © 2018 Tieme van Veen. All rights reserved.
//

import UIKit
import Alamofire

class SunriseController: RequestController {

    @IBOutlet weak var sunriseSwitch: UISwitch!
    @IBOutlet weak var timePicker: UIDatePicker!
    @IBOutlet weak var amountPicker: UISegmentedControl!

    var throttleTimer: Timer?

    override func viewWillAppear(_ animated: Bool) {
        super.viewWillAppear(animated)
        self.retrieveValues()
    }

    override func viewDidLoad() {
        super.viewDidLoad()
    }

    @IBAction func onChangeValue(_ sender: Any) {
        timePicker.isEnabled = sunriseSwitch.isOn
        amountPicker.isEnabled = sunriseSwitch.isOn

        throttleTimer?.invalidate()
        throttleTimer = Timer.scheduledTimer(
            timeInterval: 0.5,
            target: self,
            selector: #selector(sendRequest),
            userInfo: nil,
            repeats: false
        );
    }

    @objc func sendRequest() {
        self.onLoading(status: "Saving...")

        let amounts = [
            Steps.full,
            Steps.threeQuarters,
            Steps.half,
            Steps.quarter,
        ]

        let components = Calendar.current.dateComponents([.hour, .minute], from: timePicker.date)
        guard let hour = components.hour, let minutes = components.minute else {
            self.onLoading(status: "Invalid time")
            return
        }

        let url = "http://192.168.2.22/setSunrise"
        let parameters: Parameters = [
            "isEnabled": self.sunriseSwitch.isOn,
            "amount": amounts[amountPicker.selectedSegmentIndex].rawValue,
            "hour": hour,
            "minutes": minutes,
            "secondsGtmOffset": TimeZone.current.secondsFromGMT()
        ]

        Alamofire.request(url, parameters: parameters)
            .validate()
            .responseJSON { response in
//                print("request \(response.response!.statusCode) - \(response.request!.url!) \(String(describing: response.result.value))");

                switch response.result {
                case .success:
                    self.onSuccess(status: "Saved")
                    self.storeValues()
                case .failure(let err):
                    
                    self.onFailure(status: "Request failed... \(err.localizedDescription)", action: #selector(self.sendRequest))
                }
        }
    }

    func storeValues() {
        UserDefaults.standard.set(self.sunriseSwitch.isOn, forKey: "sunrise.switch")
        UserDefaults.standard.set(self.amountPicker.selectedSegmentIndex, forKey: "sunrise.amount")
        UserDefaults.standard.set(self.timePicker.date, forKey: "sunrise.time")
    }

    func retrieveValues() {
        self.sunriseSwitch.isOn = UserDefaults.standard.bool(forKey: "sunrise.switch")
        self.amountPicker.selectedSegmentIndex = UserDefaults.standard.integer(forKey: "sunrise.amount")
        if let date = UserDefaults.standard.object(forKey: "sunrise.time") as? Date {
            self.timePicker.date = date
        }

        timePicker.isEnabled = sunriseSwitch.isOn
        amountPicker.isEnabled = sunriseSwitch.isOn
    }
}
