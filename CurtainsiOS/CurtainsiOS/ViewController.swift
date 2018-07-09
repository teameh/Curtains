//
//  ViewController.swift
//  CurtainsiOS
//
//  Created by Tieme van Veen on 07/07/2018.
//  Copyright © 2018 Tieme van Veen. All rights reserved.
//

import UIKit
import Alamofire

class ViewController: UIViewController {

    @IBOutlet weak var datePicker: UIDatePicker!
    @IBOutlet weak var saveButton: UIButton!
    @IBOutlet weak var statusLog: UITextView!
    
    @IBOutlet weak var stepsSlider: UISlider!
    @IBOutlet weak var speedSlider: UISlider!
    @IBOutlet weak var accelSlider: UISlider!
    @IBOutlet weak var maxSlider: UISlider!

    @IBOutlet weak var stepsLabel: UILabel!
    @IBOutlet weak var speedLabel: UILabel!
    @IBOutlet weak var accelLabel: UILabel!
    
    override func viewDidLoad() {
        super.viewDidLoad()
        statusLog.text = "";
        
        self.onChangeSteps(self)
        self.onChangeSpeed(self)
        self.onChangeAccel(self)
        self.onMaxSliderChange(self)
    }
    
    @IBAction func onChangeSteps(_ sender: Any) {
        stepsLabel.text = String(format: "%.0f", stepsSlider.value)
    }
    
    @IBAction func onChangeSpeed(_ sender: Any) {
        speedLabel.text = String(format: "%.0f", speedSlider.value)
    }
    
    @IBAction func onChangeAccel(_ sender: Any) {
        accelLabel.text = String(format: "%.0f", accelSlider.value)
    }
    
    @IBAction func onMaxSliderChange(_ sender: Any) {
        speedSlider.maximumValue = maxSlider.value
        accelSlider.maximumValue = maxSlider.value
        
        self.onChangeSpeed(self)
        self.onChangeAccel(self)
    }

    @IBAction func onSave(_ sender: UIButton) {
        var intervalSinceNow: Int = Int(datePicker.date.timeIntervalSinceNow);
        self.statusLog.text = "Saving... \n\n";
        
        if(intervalSinceNow <= 0) {
            self.statusLog.text = self.statusLog.text + "intervalSinceNow <= 0... rotate in 5 seconds..\n\n";
            intervalSinceNow = 5;
        }
        
        let steps = String(format: "%.0f", stepsSlider.value);
        let speed = String(format: "%.0f", speedSlider.value);
        let accel = String(format: "%.0f", accelSlider.value);
        
        Alamofire.request("http://192.168.2.22/steps/\(steps)/\(speed)/\(accel)/\(intervalSinceNow)/")
            .validate()
            .responseJSON { response in
                switch response.result {
                case .success:
                    self.statusLog.text = self.statusLog.text + "Succes!\n\n";
                    if let json = response.result.value {
                        self.statusLog.text = self.statusLog.text + "Response JSON: \(json) \n\n";
                    }
                case .failure(let error):
                    self.statusLog.text = self.statusLog.text + "Error.. \(error) \n\n";
                }
            }
    }
}

