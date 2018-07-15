//
//  SunriseController.swift
//  CurtainsiOS
//
//  Created by Tieme van Veen on 15/07/2018.
//  Copyright © 2018 Tieme van Veen. All rights reserved.
//

import UIKit

class SunriseController: UIViewController {

    @IBOutlet weak var `switch`: UISwitch!
    @IBOutlet weak var timePicker: UIDatePicker!
    @IBOutlet weak var amountPicker: UISegmentedControl!
    @IBOutlet weak var statusLabel: UILabel!
    
    override func viewDidLoad() {
        super.viewDidLoad()

        // Do any additional setup after loading the view.
    }
    
    @IBAction func onChangeSwitch(_ sender: UISwitch) {
        
    }
    
    @IBAction func onChangeTime(_ sender: UIDatePicker) {
        
    }
    
    @IBAction func onChangeAmount(_ sender: UISegmentedControl) {
        
    }
}
